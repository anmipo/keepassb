/*
 * PwDatabaseV4.cpp
 *
 *  Created on: 1 Jun 2014
 *      Author: Andrei Popleteev
 */

#include "PwDatabaseV4.h"

#include <QObject>
#include <QDataStream>
#include "CryptoManager.h"
#include "sbreturn.h"
#include "husha2.h"
#include "huaes.h"
#include "db/PwUuid.h"
#include "util/Util.h"
#include "db/v4/DefsV4.h"
#include "db/v4/PwStreamUtilsV4.h"


// Cypher parameters and signatures
const QByteArray SALSA_20_ID = QByteArray("\x02\x00\x00\x00", 4);
const QByteArray AES_ID      = QByteArray("\x31\xC1\xF2\xE6\xBF\x71\x43\x50\xBE\x58\x05\x21\x6A\xFC\x5A\xFF", 16);
const QByteArray SALSA_20_INIT_VECTOR = QByteArray("\xE8\x30\x09\x4B\x97\x20\x5D\x2A");


// DB unlock stages progress percentage
const int UNLOCK_PROGRESS_INIT = 0;
const int UNLOCK_PROGRESS_HEADER_READ = 5;
const int UNLOCK_PROGRESS_KEY_TRANSFORMED = 70;
const int UNLOCK_PROGRESS_DECRYPTED = 80;
const int UNLOCK_PROGRESS_BLOCKS_READ = 90;
const int UNLOCK_PROGRESS_UNPACKED = 95;
const int UNLOCK_PROGRESS_DONE = 100;


PwHeaderV4::PwHeaderV4(QObject* parent) : QObject(parent), data() {
    initialized = false;
    transformRounds = 0;
    size = 0;
}

PwHeaderV4::~PwHeaderV4() {
    clear();
}

PwHeaderV4::ErrorCode PwHeaderV4::read(const QByteArray& dbBytes) {
    Q_ASSERT(!initialized);
    clear();

    QDataStream stream (dbBytes);
    stream.setByteOrder(QDataStream::LittleEndian);

    // check file signatures (although probably checked before)
    quint32 sign1, sign2, fileVersion;
    stream >> sign1 >> sign2 >> fileVersion;
    size += 3 * sizeof(quint32);
    if (sign1 !=  SIGNATURE_1)
        return SIGNATURE_1_MISMATCH;
    if (sign2 != SIGNATURE_2)
        return SIGNATURE_2_MISMATCH;
    if ((fileVersion & FILE_VERSION_CRITICAL_MASK) != (FILE_VERSION & FILE_VERSION_CRITICAL_MASK))
        return UNSUPPORTED_FILE_VERSION;

    qDebug("Signatures match");

    // read header fields
    quint8 fieldId;
    quint16 fSize;
    int fieldSize;
    while (true) {
        stream >> fieldId >> fSize;
        fieldSize = fSize;
        size += sizeof(fieldId) + sizeof(fSize) + fieldSize;

        if (fieldId == HEADER_END) {
            initialized = true;
            break;
        }

        char buf[fieldSize];
        stream.readRawData(buf, fieldSize);

        QByteArray fieldValue (buf, fieldSize);
        switch (fieldId) {
        case HEADER_COMMENT:
            break;
        case HEADER_CIPHER_ID:
            if (fieldValue != AES_ID)
                return NOT_AES; // Not AES cypher
            break;
        case HEADER_COMPRESSION_FLAGS:
            if (fieldValue.at(0) > 1)
                return UNKNOWN_COMPRESSION_ALGORITHM;
            break;
        case HEADER_MASTER_SEED:
            if (fieldSize != SB_SHA256_DIGEST_LEN)
                return TRANSFORM_SEED_SIZE_MISMATCH; // Master seed size is not 32 bytes
            break;
        case HEADER_TRANSFORM_SEED:
            if (fieldSize != SB_SHA256_DIGEST_LEN)
                return TRANSFORM_SEED_SIZE_MISMATCH; // Transform seed size is not 32 bytes
            break;
        case HEADER_TRANSFORM_ROUNDS:
            transformRounds = *(quint64*)fieldValue.constData();
            break;
        case HEADER_ENCRYPTION_IV:
            if (fieldSize != 16)
                return INITIAL_VECTOR_SIZE_MISMATCH; // Initial vector size is not 16 bytes
            break;
        case HEADER_PROTECTED_STREAM_KEY:
            if (fieldSize != SB_SHA256_DIGEST_LEN)
                return PROTECTED_STREAM_SIZE_MISMATCH; // Hashed protected stream key size is not 32 bytes
            break;
        case HEADER_STREAM_START_BYTES:
            break;
        case HEADER_INNER_RANDOM_STREAM_ID:
            if (fieldValue != SALSA_20_ID)
                return NOT_SALSA20; // Not Salsa20 algorithm
            break;
        //case HEADER_END: has been processed above
        default:
            qDebug() << "Strange header ID: " << fieldId;
        }
        qDebug() << "Header" << fieldId << "Value " << fieldValue.toHex();
        data.insert(fieldId, fieldValue);
    }

    const QByteArray headerBytes = dbBytes.left(size);
    if (CryptoManager::instance()->sha256(headerBytes, hash) != SB_SUCCESS)
        return HASHING_FAILED;

    return SUCCESS;
}

QString PwHeaderV4::getErrorMessage(ErrorCode errCode) {
    switch (errCode) {
    case SUCCESS:
        return "";
    case SIGNATURE_1_MISMATCH:
        // fallthrough
    case SIGNATURE_2_MISMATCH:
        return tr("Wrong database signature", "Error message when opening a database.");
    case UNSUPPORTED_FILE_VERSION:
        return tr("Unsupported database version", "Error message when opening a database.");
    case UNKNOWN_COMPRESSION_ALGORITHM:
        return tr("Unknown compression algorithm", "Error message when opening a database.");
    case NOT_AES:
        return tr("Twofish cypher is not supported", "Error message when opening a database. 'Twofish' is an algorithm name, do not translate it.");
    default:
        return tr("Header error", "Error message when opening a database. 'Header' refers to supplemental data placed at the beginning of a file.");
    }
}

void PwHeaderV4::clear() {
    data.clear();
    size = 0;
    transformRounds = 0;
    Util::safeClear(hash);
    initialized = false;
}

quint64 PwHeaderV4::getTransformRounds() const {
    return transformRounds;
}

QByteArray PwHeaderV4::getTransformSeed() const {
    return data.value(HEADER_TRANSFORM_SEED);
}

QByteArray PwHeaderV4::getMasterSeed() const {
    return data.value(HEADER_MASTER_SEED);
}

QByteArray PwHeaderV4::getInitialVector() const {
    return data.value(HEADER_ENCRYPTION_IV);
}

QByteArray PwHeaderV4::getStreamStartBytes() const {
    return data.value(HEADER_STREAM_START_BYTES);
}

QByteArray PwHeaderV4::getProtectedStreamKey() const {
    return data.value(HEADER_PROTECTED_STREAM_KEY);
}

bool PwHeaderV4::isCompressed() const {
    return (data.value(HEADER_COMPRESSION_FLAGS).at(0) != 0);
}

QByteArray PwHeaderV4::getHash() const {
    return hash;
}
int PwHeaderV4::sizeInBytes() const {
    return size;
}


/****************************/

PwDatabaseV4::PwDatabaseV4(QObject* parent) :
        PwDatabase(parent),
        header(),
        meta(),
        combinedKey(SB_SHA256_DIGEST_LEN, 0),
        aesKey(SB_SHA256_DIGEST_LEN, 0),
        salsa20() {
    header.setParent(this);
    meta.setParent(this);
}

PwDatabaseV4::~PwDatabaseV4() {
    // nothing to do here
}

/**
 * Returns the Backup group of this database.
 * If createIfMissing is true, creates the group if it is missing.
 * (However, if backup is disabled will not create anything and still return NULL).
 */
PwGroup* PwDatabaseV4::getBackupGroup(bool createIfMissing) {
    // TODO implement PwDatabaseV4::getBackupGroup
    Q_UNUSED(createIfMissing); // remove when method is implemented
    return NULL;
}

bool PwDatabaseV4::isSignatureMatch(const QByteArray& rawDbData) {
    QDataStream stream (rawDbData);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint32 sign1, sign2;
    stream >> sign1 >> sign2;

    return (sign1 ==  PwHeaderV4::SIGNATURE_1) && (sign2 == PwHeaderV4::SIGNATURE_2);
}

void PwDatabaseV4::clear() {
    header.clear();
    meta.clear();
    Util::safeClear(combinedKey);
    Util::safeClear(aesKey);
    PwDatabase::clear(); // ancestor's cleaning
}

void PwDatabaseV4::load(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) {
    if (!buildCompositeKey(password.toUtf8(), keyFileData, combinedKey)) {
        emit dbLoadError(tr("Cryptographic library error", "Generic error message from a cryptographic library"), COMPOSITE_KEY_ERROR);
        return;
    }

    if (readDatabase(dbFileData))
        emit dbUnlocked();
}

/**
 * Extracts the key from a correctly-formed XML file.
 * Returns true if successful, false otherwise.
 */
bool PwDatabaseV4::processXmlKeyFile(const QByteArray& keyFileData, QByteArray& key) const {
    QXmlStreamReader xml(keyFileData);
    if (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE)) {
            if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE_META)) {
                xml.skipCurrentElement(); // skip the Meta element
                if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE_KEY)) {
                    if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE_DATA)) {
                        // QString keyText = xml.readElementText();
                        // key = QByteArray::fromBase64(keyText.toLatin1());
                        key = PwStreamUtilsV4::readBase64(xml);
                        if (!xml.hasError()) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    xml.clear();
    return false;
}

bool PwDatabaseV4::buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const {
    CryptoManager* cm = CryptoManager::instance();

    QByteArray preKey;
    int ec;
    // if no key file were supplied, the keyFileData will be empty
    if (!passwordKey.isEmpty() && !keyFileData.isEmpty()) {
        qDebug() << "using password and key file";
        ec = cm->sha256(passwordKey, preKey);
        if (ec != SB_SUCCESS)
            return false;

        QByteArray fKey;
        if (!processKeyFile(keyFileData, fKey))
            return false;
        preKey.append(fKey);
    } else if (keyFileData.isEmpty()) {
        qDebug() << "using password only";
        int ec = cm->sha256(passwordKey, preKey);
        if (ec != SB_SUCCESS)
            return false;
    } else if (passwordKey.isEmpty()) {
        qDebug() << "using key file only";
        if (!processKeyFile(keyFileData, preKey))
            return false;
    } else {
        qDebug() << "empty keys provided (should not happen)";
        return false;
    }

    ec = cm->sha256(preKey, combinedKey);
    Util::safeClear(preKey);
    if (ec != SB_SUCCESS)
        return false;

    return true;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::transformKey(const PwHeaderV4& header, const QByteArray& combinedKey, QByteArray& aesKey,
        const int progressFrom, const int progressTo) {
//    aesKey.clear();

    CryptoManager* cm = CryptoManager::instance();

    QByteArray subKey1 = combinedKey.left(16);
    QByteArray subKey2 = combinedKey.right(16);
    QByteArray key = header.getTransformSeed();
    quint64 transformRounds = header.getTransformRounds();

    // temporary arrays for storing intermediate keys
    QByteArray subKey1bis(subKey1.length(), 0);
    QByteArray subKey2bis(subKey2.length(), 0);

    int progress = progressFrom;
    int subProgress = 0;
    int subProgressThreshold = ceil(transformRounds / (progressTo - progressFrom));
    int ec;

    // prepare key transform
    if (cm->beginKeyTransform(key, SB_AES_128_KEY_BYTES) != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_INIT_ERROR;

    unsigned char* origKey1 = reinterpret_cast<unsigned char*>(subKey1.data());
    unsigned char* origKey2 = reinterpret_cast<unsigned char*>(subKey2.data());
    unsigned char* transKey1 = reinterpret_cast<unsigned char*>(subKey1bis.data());
    unsigned char* transKey2 = reinterpret_cast<unsigned char*>(subKey2bis.data());
    for (quint64 round = 0; round < transformRounds; round++) {
        ec = cm->performKeyTransform(origKey1, transKey1);
        memcpy(origKey1, transKey1, SB_AES_128_BLOCK_BYTES);
        if (ec != SB_SUCCESS) break;

        ec = cm->performKeyTransform(origKey2, transKey2);
        memcpy(origKey2, transKey2, SB_AES_128_BLOCK_BYTES);
        if (ec != SB_SUCCESS) break;

        if (++subProgress > subProgressThreshold) {
            subProgress = 0;
            progress++;
            emit progressChanged(progress);
        }
    }
    if (ec != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_ERROR_1;

    if (cm->endKeyTransform() != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_END_ERROR;


    QByteArray step2key;
    step2key.append(subKey1);
    step2key.append(subKey2);
    QByteArray transformedKey;
    ec = cm->sha256(step2key, transformedKey);
    Util::safeClear(subKey1);
    Util::safeClear(subKey2);
    Util::safeClear(step2key);
    if (ec != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_ERROR_2;

    QByteArray step3key(header.getMasterSeed());
    step3key.append(transformedKey);
    ec = cm->sha256(step3key, aesKey);
    Util::safeClear(transformedKey);
    Util::safeClear(step3key);
    if (ec != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_ERROR_3;

    return ErrorCodesV4::SUCCESS;
}

bool PwDatabaseV4::readDatabase(const QByteArray& dbBytes) {
    emit progressChanged(UNLOCK_PROGRESS_INIT);

    PwHeaderV4::ErrorCode headerErrCode = header.read(dbBytes);
    if (headerErrCode != PwHeaderV4::SUCCESS) {
        qDebug() << PwHeaderV4::getErrorMessage(headerErrCode) << headerErrCode;
        emit dbLoadError(PwHeaderV4::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_HEADER_READ);

    /* Calculate the AES key */
    ErrorCodesV4::ErrorCode err = transformKey(header, combinedKey, aesKey, UNLOCK_PROGRESS_HEADER_READ, UNLOCK_PROGRESS_KEY_TRANSFORMED);
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - transformKey" << err;
        emit dbLoadError(tr("Cannot decrypt database", "A generic error message"), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Decrypt data */
    int dataSize = dbBytes.size() - header.sizeInBytes();
    QByteArray decryptedData (dataSize, 0);
    // DB header not needed for decryption
    QByteArray dbBytesWithoutHeader = dbBytes.right(dataSize);
    err = decryptData(dbBytesWithoutHeader, decryptedData);
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - decryptData" << err;
        emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        return false;
    }
    emit progressChanged(UNLOCK_PROGRESS_DECRYPTED);

    QDataStream decryptedStream(decryptedData);
    decryptedStream.setByteOrder(QDataStream::LittleEndian);

    /* Verify first bytes */
    const int N = SB_SHA256_DIGEST_LEN;
    QByteArray startBytes(N, 0);
    decryptedStream.readRawData(startBytes.data(), N);
    if (startBytes != header.getStreamStartBytes()) {
        qDebug() << "First bytes do not match" << err;
        emit invalidPasswordOrKey();
        return false;
    }

    /* Read data blocks */
    QByteArray blocksData;
    err = readBlocks(decryptedStream, blocksData);
    Util::safeClear(decryptedData); // not needed any further
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - readBlocks" << err;
        emit dbLoadError(tr("Error reading database", "An error message"), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_BLOCKS_READ);

    QByteArray xmlData;
    if (header.isCompressed()) {
        /* Inflate GZip data to XML */
        Util::ErrorCode inflateErr = Util::inflateGZipData(blocksData, xmlData);
        Util::safeClear(blocksData);
        if (inflateErr != Util::SUCCESS) {
            qDebug() << "Error inflating database";
            emit dbLoadError(tr("Error inflating database", "An error message. Inflating means decompression of compressed data."), inflateErr);
            return false;
        }
    } else {
        xmlData = blocksData;
    }

    emit progressChanged(UNLOCK_PROGRESS_UNPACKED);

    /* Init Salsa20 for reading protected values */
    err = initSalsa20();
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - initSalsa20" << err;
        emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        return false;
    }

    /* Parse XML */
    QString xmlString = QString::fromUtf8(xmlData.data(), xmlData.size());
    err = parseXml(xmlString);
    Util::safeClear(xmlData);
    Util::safeClear(xmlString);
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Error parsing database" << err;
        emit dbLoadError(tr("Cannot parse database", "An error message. Parsing refers to the analysis/understanding of file content (do not confuse with reading it)."), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_DONE);
    qDebug() << "DB unlocked";

    return true;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::initSalsa20() {
    CryptoManager* cm = CryptoManager::instance();

    QByteArray salsaKey;
    int err = cm->sha256(header.getProtectedStreamKey(), salsaKey);
    if (err != SB_SUCCESS)
        return ErrorCodesV4::CANNOT_INIT_SALSA20;

    salsa20.init(salsaKey, SALSA_20_INIT_VECTOR);
    return ErrorCodesV4::SUCCESS;
}

/**
 * Decrypts the DB's data using current keys.
 */
ErrorCodesV4::ErrorCode PwDatabaseV4::decryptData(const QByteArray& encryptedData, QByteArray& decryptedData) {
    // assert encryptedData.size is multiple of 16 bytes

    CryptoManager* cm = CryptoManager::instance();
    int err = cm->decryptAES(aesKey, header.getInitialVector(), encryptedData, decryptedData);
    if (err != SB_SUCCESS) {
        qDebug() << "decryptAES error: " << err;
        return ErrorCodesV4::CANNOT_DECRYPT_DB;
    }
    return ErrorCodesV4::SUCCESS;
}

/**
 * Extracts data blocks from the decrypted data stream, verifying hashes.
 */
ErrorCodesV4::ErrorCode PwDatabaseV4::readBlocks(QDataStream& inputStream, QByteArray& blocksData) {
    quint32 blockSize, readBlockId;
    QByteArray blockHash(SB_SHA256_DIGEST_LEN, 0);
    QByteArray computedHash(SB_SHA256_DIGEST_LEN, 0);

    Util::safeClear(blocksData);
    CryptoManager* cm = CryptoManager::instance();

    quint32 blockId = 0;
    while (true) {
        inputStream >> readBlockId;
        if (readBlockId != blockId) {
            qDebug() << "readBlocks wrong block ID";
            return ErrorCodesV4::WRONG_BLOCK_ID;
        }
        blockId++;

        inputStream.readRawData(blockHash.data(), SB_SHA256_DIGEST_LEN);
        inputStream >> blockSize;
        if (blockSize == 0) {
            if (Util::isAllZero(blockHash)) {
                break;
            } else {
                qDebug() << "readBlocks block hash is not all-zeros";
                return ErrorCodesV4::BLOCK_HASH_NON_ZERO;
            }
        }
        QByteArray blockData(blockSize, 0);
        inputStream.readRawData(blockData.data(), blockSize);
        int err = cm->sha256(blockData, computedHash);
        if ((err != SB_SUCCESS) || (computedHash != blockHash)) {
            qDebug() << "readBlocks block hash mismatch";
            return ErrorCodesV4::BLOCK_HASH_MISMATCH;
        }
        blocksData.append(blockData);
        Util::safeClear(blockData);
    }
    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::parseXml(const QString& xmlString) {
    if (_rootGroup) {
        delete _rootGroup;
        _rootGroup = NULL;
    }

    PwGroupV4* rootV4 = new PwGroupV4(this);
    rootV4->setDatabase(this);
    rootV4->setParentGroup(NULL); // not Qt parent, but the group containing this one

    ErrorCodesV4::ErrorCode err;
    QXmlStreamReader xml(xmlString);
    QStringRef tagName;
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNextStartElement()) {
            tagName = xml.name();
            if (tagName == XML_META) {
                err = meta.readFromStream(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
                if (!meta.isHeaderHashMatch(header.getHash()))
                    return ErrorCodesV4::XML_META_HEADER_HASH_MISMATCH;

            } else if (tagName == XML_ROOT) {
                if (xml.readNextStartElement() && (xml.name() == XML_GROUP)) {
                    err = rootV4->readFromStream(xml, meta, salsa20);
                    if (err != ErrorCodesV4::SUCCESS)
                        return err;
                } else {
                    qDebug() << "SEVERE: there is no group in the root";
                    return ErrorCodesV4::XML_NO_ROOT_GROUP;
                }
            }
        }
    }

    xml.clear();
    if (xml.hasError())
        return ErrorCodesV4::XML_PARSING_ERROR;

    _rootGroup = rootV4;
//    debugPrint(_rootGroup, 2);
    return ErrorCodesV4::SUCCESS;
}


/**
 * Encrypts and writes DB content to the given array.
 */
bool PwDatabaseV4::save(QByteArray& outData) {
    //TODO implement V4 saving
    Q_UNUSED(outData); // remove when method is implemented
    return false;
}
