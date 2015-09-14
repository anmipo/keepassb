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

// Size of encryption initial vector in bytes
const int INITIAL_VECTOR_SIZE = 16;

// Default number of transform rounds for new DBs
const quint64 DEFAULT_TRANSFORM_ROUNDS = 123456; // ~2 seconds delay on BB Q10

// DB unlock stages progress percentage
const quint8 UNLOCK_PROGRESS_KEY_TRANSFORM[]  = { 0, 60};
const quint8 UNLOCK_PROGRESS_DECRYPTION[]     = {60, 70};
const quint8 UNLOCK_PROGRESS_READ_BLOCKS[]    = {70, 75};
const quint8 UNLOCK_PROGRESS_GZIP_INFLATE[]   = {75, 80};
const quint8 UNLOCK_PROGRESS_XML_PARSING[]    = {80, 100};

// DB save stages progress percentage
const quint8 SAVE_PROGRESS_KEY_TRANSFORM[]  = { 0, 65};
const quint8 SAVE_PROGRESS_XML_WRITING[]    = {65, 80};
const quint8 SAVE_PROGRESS_GZIP_DEFLATE[]   = {80, 85};
const quint8 SAVE_PROGRESS_WRITE_BLOCKS[]   = {85, 90};
const quint8 SAVE_PROGRESS_ENCRYPTION[]     = {90, 100};

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

    LOG("Signatures match");

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
            if (Util::bytesToQuint32(fieldValue) > COMPRESSION_GZIP)
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
            transformRounds = Util::bytesToQuint64(fieldValue);
            break;
        case HEADER_ENCRYPTION_IV:
            if (fieldSize != INITIAL_VECTOR_SIZE)
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
            LOG("Strange header ID: %d", fieldId);
        }
        data.insert(fieldId, fieldValue);
    }

    const QByteArray headerBytes = dbBytes.left(size);
    if (CryptoManager::instance()->sha256(headerBytes, hash) != SB_SUCCESS)
        return HASHING_FAILED;

    return SUCCESS;
}

PwHeaderV4::ErrorCode PwHeaderV4::write(QDataStream& outStream) {
    QByteArray headerBytes;
    QDataStream stream(&headerBytes, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << SIGNATURE_1 << SIGNATURE_2 << FILE_VERSION;

    writeHeaderField(stream, HEADER_CIPHER_ID);
    writeHeaderField(stream, HEADER_COMPRESSION_FLAGS);
    writeHeaderField(stream, HEADER_MASTER_SEED);
    writeHeaderField(stream, HEADER_TRANSFORM_SEED);
    writeHeaderField(stream, HEADER_TRANSFORM_ROUNDS);
    writeHeaderField(stream, HEADER_ENCRYPTION_IV);
    writeHeaderField(stream, HEADER_PROTECTED_STREAM_KEY);
    writeHeaderField(stream, HEADER_STREAM_START_BYTES);
    writeHeaderField(stream, HEADER_INNER_RANDOM_STREAM_ID);
    writeHeaderField(stream, HEADER_END);

    // update header hash value
    if (CryptoManager::instance()->sha256(headerBytes, hash) != SB_SUCCESS)
        return HASHING_FAILED;

    outStream.writeRawData(headerBytes.constData(), headerBytes.size());
    return SUCCESS;
}

void PwHeaderV4::writeHeaderField(QDataStream& stream, quint8 fieldId) {
    quint16 fieldSize = 0;
    stream << fieldId;
    if (data.contains(fieldId)) {
        QByteArray fieldValue = data.value(fieldId);
        fieldSize = fieldValue.size();
        stream << fieldSize;
        stream.writeRawData(fieldValue.constData(), fieldValue.size());
    } else {
        stream << fieldSize;
    }
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

/**
 * Resets encryption seeds to random values.
 * Returns false in case of RNG error;
 */
bool PwHeaderV4::randomizeInitialVectors() {
    CryptoManager* cm = CryptoManager::instance();

    QByteArray masterSeed;
    QByteArray transformSeed;
    QByteArray initialVector;
    QByteArray protectedStreamKey;
    QByteArray streamStartBytes;

    if (cm->getRandomBytes(masterSeed, SB_SHA256_DIGEST_LEN) != SB_SUCCESS)
        return false;

    if (cm->getRandomBytes(transformSeed, SB_SHA256_DIGEST_LEN) != SB_SUCCESS)
        return false;

    if (cm->getRandomBytes(initialVector, INITIAL_VECTOR_SIZE) != SB_SUCCESS)
        return false;

    if (cm->getRandomBytes(protectedStreamKey, SB_SHA256_DIGEST_LEN) != SB_SUCCESS)
        return false;

    if (cm->getRandomBytes(streamStartBytes, SB_SHA256_DIGEST_LEN) != SB_SUCCESS)
        return false;

    data.insert(HEADER_MASTER_SEED, masterSeed);
    data.insert(HEADER_TRANSFORM_SEED, transformSeed);
    data.insert(HEADER_ENCRYPTION_IV, initialVector);
    data.insert(HEADER_PROTECTED_STREAM_KEY, protectedStreamKey);
    data.insert(HEADER_STREAM_START_BYTES, streamStartBytes);
    return true;
}

quint64 PwHeaderV4::getTransformRounds() const {
    return transformRounds;
}

void PwHeaderV4::setTransformRounds(const quint64 value) {
    data.insert(HEADER_TRANSFORM_ROUNDS, Util::quint64ToBytes(value));
    transformRounds = value;
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
    quint32 flags = Util::bytesToQuint32(data.value(HEADER_COMPRESSION_FLAGS));
    return (flags != COMPRESSION_NONE);
}

void PwHeaderV4::setCompressionFlags(const quint32 flags) {
    data.insert(HEADER_COMPRESSION_FLAGS, Util::quint32ToBytes(flags));
}

QByteArray PwHeaderV4::getHash() const {
    return hash;
}

void PwHeaderV4::setCipherId(const PwUuid& uuid) {
    data.insert(HEADER_CIPHER_ID, uuid.toByteArray());
}

void PwHeaderV4::setInnerRandomStreamId(const PwUuid& uuid) {
    data.insert(HEADER_INNER_RANDOM_STREAM_ID, uuid.toByteArray());
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
        salsa20(),
        deletedObjects() {
    header.setParent(this);
    meta.setParent(this);
}

PwDatabaseV4::~PwDatabaseV4() {
    clear();
}

/**
 * Returns the Backup group of this database.
 * If createIfMissing is true, creates the group if it is missing.
 * (However, if backup is disabled will not create anything and still return NULL).
 */
PwGroup* PwDatabaseV4::getBackupGroup(bool createIfMissing) {
    PwGroup* recycleBinGroup = NULL;
    if (meta.isRecycleBinEnabled()) {
        PwUuid recycleBinGroupUuid = meta.getRecycleBinGroupUuid();
        PwGroupV4* root = dynamic_cast<PwGroupV4*>(getRootGroup());

        // search only if UUID is set to something meaningful
        if (!recycleBinGroupUuid.isEmpty() && !recycleBinGroupUuid.isAllZero())
            recycleBinGroup = root->findGroupByUuid(recycleBinGroupUuid);

        if (!recycleBinGroup && createIfMissing) {
            // no such group - create one
            recycleBinGroup = root->createGroup();
            QString recycleBinGroupName = tr("Recycle Bin", "Name of a group which contains deleted entries");
            recycleBinGroup->setName(recycleBinGroupName);
            recycleBinGroup->setIconId(PwIcon::TRASH_BIN);
            recycleBinGroup->setDeleted(true);

            meta.setRecycleBinGroupUuid(recycleBinGroup->getUuid());
            meta.setRecycleBinChangedTime(QDateTime::currentDateTime());
        }
    }
    return recycleBinGroup;
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

    qDeleteAll(deletedObjects);
    deletedObjects.clear();

    PwDatabase::clear(); // ancestor's cleaning
}

/** Adds an item to the DeletedObjects list. */
void PwDatabaseV4::addDeletedObject(const PwUuid& uuid) {
    PwDeletedObject* deletedObject = new PwDeletedObject(uuid);
    deletedObjects.append(deletedObject);
}

void PwDatabaseV4::load(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) {
    if (!buildCompositeKey(getPasswordBytes(password), keyFileData, combinedKey)) {
        emit dbLoadError(tr("Cryptographic library error", "Generic error message from a cryptographic library"), COMPOSITE_KEY_ERROR);
        return;
    }

    if (readDatabase(dbFileData)) {
        emit dbUnlocked();
    } else {
        // This is needed to clean up at higher abstraction levels
        emit dbLocked();
    }
}

/** Converts the password string to its raw representation, as of format version's rules */
QByteArray PwDatabaseV4::getPasswordBytes(const QString& password) const {
    return password.toUtf8();
}

/** Setter for the combinedKey field */
void PwDatabaseV4::setCombinedKey(const QByteArray& newKey) {
    combinedKey = Util::deepCopy(newKey);
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
        LOG("using password and key file");
        ec = cm->sha256(passwordKey, preKey);
        if (ec != SB_SUCCESS)
            return false;

        QByteArray fKey;
        if (!processKeyFile(keyFileData, fKey))
            return false;
        preKey.append(fKey);
    } else if (keyFileData.isEmpty()) {
        LOG("using password only");
        int ec = cm->sha256(passwordKey, preKey);
        if (ec != SB_SUCCESS)
            return false;
    } else if (passwordKey.isEmpty()) {
        LOG("using key file only");
        if (!processKeyFile(keyFileData, preKey))
            return false;
    } else {
        LOG("empty keys provided (should not happen)");
        return false;
    }

    ec = cm->sha256(preKey, combinedKey);
    Util::safeClear(preKey);
    if (ec != SB_SUCCESS)
        return false;

    return true;
}

bool PwDatabaseV4::readDatabase(const QByteArray& dbBytes) {
    /* Read DB header */
    PwHeaderV4::ErrorCode headerErrCode = header.read(dbBytes);
    if (headerErrCode != PwHeaderV4::SUCCESS) {
        LOG("%s: %d", PwHeaderV4::getErrorMessage(headerErrCode).toUtf8().constData(), headerErrCode);
        emit dbLoadError(PwHeaderV4::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }

    /* Calculate the AES key */
    setPhaseProgressBounds(UNLOCK_PROGRESS_KEY_TRANSFORM);
    PwDatabase::ErrorCode dbErr = transformKey(header.getMasterSeed(), header.getTransformSeed(),
            header.getTransformRounds(), combinedKey, aesKey);
    if (dbErr != PwDatabase::SUCCESS) {
        LOG("Cannot decrypt database - transformKey: %d", dbErr);
        emit dbLoadError(tr("Cannot decrypt database", "A generic error message"), dbErr);
        return false;
    }

    /* Decrypt data */
    setPhaseProgressBounds(UNLOCK_PROGRESS_DECRYPTION);
    int dataSize = dbBytes.size() - header.sizeInBytes();
    QByteArray decryptedData (dataSize, 0);
    // DB header not needed for decryption
    QByteArray dbBytesWithoutHeader = dbBytes.right(dataSize);
    ErrorCodesV4::ErrorCode err = decryptData(dbBytesWithoutHeader, decryptedData);
    if (err != ErrorCodesV4::SUCCESS) {
        LOG("Cannot decrypt database - decryptData: %d", err);
        emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        return false;
    }

    QDataStream decryptedStream(decryptedData);
    decryptedStream.setByteOrder(QDataStream::LittleEndian);

    /* Verify first bytes */
    const int VERIFICATION_LENGTH = SB_SHA256_DIGEST_LEN;
    QByteArray startBytes(VERIFICATION_LENGTH, 0);
    decryptedStream.readRawData(startBytes.data(), VERIFICATION_LENGTH);
    if (startBytes != header.getStreamStartBytes()) {
        LOG("First bytes do not match: %d", err);
        emit invalidPasswordOrKey();
        return false;
    }

    /* Read data blocks */
    setPhaseProgressBounds(UNLOCK_PROGRESS_READ_BLOCKS);
    QByteArray blocksData;
    int streamSize = decryptedData.size() - VERIFICATION_LENGTH;
    err = readBlocks(decryptedStream, streamSize, blocksData);
    Util::safeClear(decryptedData); // not needed any further
    if (err != ErrorCodesV4::SUCCESS) {
        LOG("Cannot decrypt database - readBlocks: %d", err);
        emit dbLoadError(tr("Error reading database", "An error message"), err);
        return false;
    }


    QByteArray xmlData;
    if (header.isCompressed()) {
        /* Inflate GZip data to XML */
        setPhaseProgressBounds(UNLOCK_PROGRESS_GZIP_INFLATE);
        Util::ErrorCode inflateErr = Util::inflateGZipData(blocksData, xmlData, this);
        Util::safeClear(blocksData);
        if (inflateErr != Util::SUCCESS) {
            LOG("Error inflating database");
            emit dbLoadError(tr("Error inflating database", "An error message. Inflating means decompression of compressed data."), inflateErr);
            return false;
        }
    } else {
        xmlData = blocksData;
    }


    /* Init Salsa20 for reading protected values */
    err = initSalsa20();
    if (err != ErrorCodesV4::SUCCESS) {
        LOG("Cannot decrypt database - initSalsa20: %d", err);
        emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        return false;
    }

    /* Parse XML */
    setPhaseProgressBounds(UNLOCK_PROGRESS_XML_PARSING);
    QString xmlString = QString::fromUtf8(xmlData.data(), xmlData.size());
    err = parseXml(xmlString);
    Util::safeClear(xmlData);
    Util::safeClear(xmlString);
    if (err != ErrorCodesV4::SUCCESS) {
        LOG("Error parsing database: %d", err);
        emit dbLoadError(tr("Cannot parse database", "An error message. Parsing refers to the analysis/understanding of file content (do not confuse with reading it)."), err);
        return false;
    }
    LOG("DB unlocked");

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
    int err = cm->decryptAES(aesKey, header.getInitialVector(), encryptedData, decryptedData, this);
    if (err != SB_SUCCESS) {
        LOG("decryptAES error: %d", err);
        return ErrorCodesV4::CANNOT_DECRYPT_DB;
    }
    return ErrorCodesV4::SUCCESS;
}

/**
 * Extracts data blocks from the decrypted data stream, verifying hashes.
 */
ErrorCodesV4::ErrorCode PwDatabaseV4::readBlocks(QDataStream& inputStream, const int streamSize, QByteArray& blocksData) {
    quint32 blockSize, readBlockId;
    QByteArray blockHash(SB_SHA256_DIGEST_LEN, 0);
    QByteArray computedHash(SB_SHA256_DIGEST_LEN, 0);

    setPhaseProgressRawTarget(streamSize);

    Util::safeClear(blocksData);
    CryptoManager* cm = CryptoManager::instance();

    quint32 blockId = 0;
    while (true) {
        inputStream >> readBlockId;
        if (readBlockId != blockId) {
            LOG("readBlocks wrong block ID");
            return ErrorCodesV4::WRONG_BLOCK_ID;
        }
        blockId++;

        inputStream.readRawData(blockHash.data(), SB_SHA256_DIGEST_LEN);
        inputStream >> blockSize;
        if (blockSize == 0) {
            if (Util::isAllZero(blockHash)) {
                break;
            } else {
                LOG("readBlocks block hash is not all-zeros");
                return ErrorCodesV4::BLOCK_HASH_NON_ZERO;
            }
        }
        QByteArray blockData(blockSize, 0);
        inputStream.readRawData(blockData.data(), blockSize);
        int err = cm->sha256(blockData, computedHash);
        if ((err != SB_SUCCESS) || (computedHash != blockHash)) {
            LOG("readBlocks block hash mismatch");
            return ErrorCodesV4::BLOCK_HASH_MISMATCH;
        }
        blocksData.append(blockData);
        Util::safeClear(blockData);

        int bytesRead = sizeof(readBlockId) + SB_SHA256_DIGEST_LEN + sizeof(blockSize) + blockSize;
        increaseProgress(bytesRead);
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

    setPhaseProgressRawTarget(xmlString.size());

    QXmlStreamReader xml(xmlString);
    if (xml.readNextStartElement() && (xml.name() == XML_KEEPASS_FILE)) {
        ErrorCodesV4::ErrorCode err = parseXmlDocumentTag(xml, *rootV4);
        if (err != ErrorCodesV4::SUCCESS)
            return err;
    } else {
        LOG("XML document tag is not <KeePassFile>: %s", xml.name().toUtf8().constData());
        return ErrorCodesV4::XML_DOCUMENT_PARSING_ERROR_NOT_KEEPASS;
    }

    bool xmlHadError = xml.hasError();
    xml.clear();
    if (xmlHadError)
        return ErrorCodesV4::XML_PARSING_ERROR_GENERIC;

    _rootGroup = rootV4;
    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::parseXmlDocumentTag(QXmlStreamReader& xml, PwGroupV4& root) {
    Q_ASSERT(xml.name() == XML_KEEPASS_FILE);

    ErrorCodesV4::ErrorCode err;

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_KEEPASS_FILE == tagName))) {
        if (xml.isStartElement()) {
            if (tagName == XML_META) {
                err = meta.readFromStream(xml, salsa20);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
                if (!meta.isHeaderHashMatch(header.getHash()))
                    return ErrorCodesV4::XML_META_HEADER_HASH_MISMATCH;

            } else if (tagName == XML_ROOT) {
                err = parseRoot(xml, root);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;

            } else {
                LOG("unknown tag in XML document: %s", tagName.toUtf8().constData());
                return ErrorCodesV4::XML_DOCUMENT_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError())
        return ErrorCodesV4::XML_DOCUMENT_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::parseRoot(QXmlStreamReader& xml, PwGroupV4& root) {
    Q_ASSERT(xml.name() == XML_ROOT);

    ErrorCodesV4::ErrorCode err;

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_ROOT == tagName))) {
        if (xml.isStartElement()) {
            if (XML_GROUP == tagName) {
                err = root.readFromStream(xml, meta, salsa20, this);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (XML_DELETED_OBJECTS == tagName) {
                err = parseDeletedObjects(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else {
                LOG("unknown tag in the Root: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_ROOT_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_ROOT_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;
}
ErrorCodesV4::ErrorCode PwDatabaseV4::parseDeletedObjects(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_DELETED_OBJECTS);

    ErrorCodesV4::ErrorCode err;

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_DELETED_OBJECTS == tagName))) {
        if (xml.isStartElement()) {
            if (XML_DELETED_OBJECT_ITEM == tagName) {
                PwDeletedObject* deletedObject = new PwDeletedObject(this);
                err = deletedObject->readFromStream(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else {
                LOG("unknown tag within DeletedObjects: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_DELETED_OBJECTS_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError())
        return ErrorCodesV4::XML_DELETED_OBJECTS_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;
}

/**
 * Encrypts and writes DB content to the given array.
 */
bool PwDatabaseV4::save(QByteArray& outData) {
    QString saveErrorMessage = tr("Cannot save database", "An error message");
    outData.clear();

    // Randomize encryption seeds
    if (!header.randomizeInitialVectors()) {
        LOG("PwDatabaseV4::save() failed to randomize header seeds");
        emit dbSaveError(saveErrorMessage, ErrorCodesV4::RNG_ERROR_1);
        return false;
    }

    setPhaseProgressBounds(SAVE_PROGRESS_KEY_TRANSFORM);
    PwDatabase::ErrorCode dbErr = transformKey(header.getMasterSeed(), header.getTransformSeed(),
            header.getTransformRounds(), combinedKey, aesKey);
    if (dbErr != PwDatabase::SUCCESS) {
        LOG("transformKey error while saving: %d", dbErr);
        emit dbSaveError(saveErrorMessage, dbErr);
        return false;
    }


    // Reset Salsa20 state and apply new keys
    initSalsa20();

    QDataStream outStream(&outData, QIODevice::WriteOnly);
    outStream.setByteOrder(QDataStream::LittleEndian);

    // write the header, this implicitly updates header's hash
    PwHeaderV4::ErrorCode headerErr = header.write(outStream);
    if (headerErr != PwHeaderV4::SUCCESS) {
        LOG("error writing PwHeaderV4: %d", headerErr);
        emit dbSaveError(saveErrorMessage, headerErr);
        return false;
    }

    // Update Meta data
    meta.setHeaderHash(header.getHash());
    meta.updateBinaries(dynamic_cast<PwGroupV4*>(getRootGroup())); // Rebuild binary pool from attachments


    // Prepare XML content
    setPhaseProgressBounds(SAVE_PROGRESS_XML_WRITING);
    setPhaseProgressRawTarget(1);

    QByteArray xmlContentData;
    QXmlStreamWriter xml(&xmlContentData);
    xml.setCodec("UTF-8");

    // KeePass 2 uses pretty-printed XML in DBs, so shall we.
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1); // one tab

    xml.writeStartDocument("1.0", true);
    xml.writeStartElement(XML_KEEPASS_FILE);
    ErrorCodesV4::ErrorCode err = meta.writeToStream(xml, salsa20);
    if (err != ErrorCodesV4::SUCCESS) {
        LOG("failed to write Meta to XML: %d", err);
        emit dbSaveError(saveErrorMessage, err);
        return false;
    }

    xml.writeStartElement(XML_ROOT);
    //write groups
    setPhaseProgressRawTarget(countAllChildren());
    PwGroupV4* root = dynamic_cast<PwGroupV4*>(getRootGroup());
    root->writeToStream(xml, meta, salsa20, this);

    //write DeletedObjects
    if (deletedObjects.isEmpty()) {
        xml.writeEmptyElement(XML_DELETED_OBJECTS);
    } else {
        xml.writeStartElement(XML_DELETED_OBJECTS);
        for (int i = 0; i < deletedObjects.size(); i++) {
            deletedObjects.at(i)->writeToStream(xml);
        }
        xml.writeEndElement(); // XML_DELETED_OBJECTS
    }

    xml.writeEndElement(); // XML_ROOT
    xml.writeEndElement(); // XML_KEEPASS_FILE
    xml.writeEndDocument();

    // compress the data if necessary
    setPhaseProgressBounds(SAVE_PROGRESS_GZIP_DEFLATE);
    QByteArray dataToSplit;
    if (header.isCompressed()) {
        QByteArray gzipData;
        Util::ErrorCode gzErr = Util::compressToGZip(xmlContentData, gzipData, this);
        Util::safeClear(xmlContentData);
        if (gzErr != Util::SUCCESS) {
            LOG("PwDatabaseV4::save() gzip compression failed: %d", gzErr);
            emit dbSaveError(saveErrorMessage, ErrorCodesV4::GZIP_COMPRESS_ERROR);
            return false;
        }
        dataToSplit = gzipData;
    } else {
        dataToSplit = xmlContentData;
    }

    // split data to hashed blocks
    setPhaseProgressBounds(SAVE_PROGRESS_WRITE_BLOCKS);
    QByteArray dataInBlocks;
    QDataStream blockStream(&dataInBlocks, QIODevice::WriteOnly);
    blockStream.setByteOrder(QDataStream::LittleEndian);

    // random stream start bytes must go before any blocks
    QByteArray streamStartBytes = header.getStreamStartBytes();
    blockStream.writeRawData(streamStartBytes.constData(), streamStartBytes.size());

    err = splitToBlocks(dataToSplit, blockStream);
    Util::safeClear(dataToSplit);
    if (err != ErrorCodesV4::SUCCESS) {
        LOG("PwDatabaseV4::save() failed to make hashed blocks: %d", err);
        emit dbSaveError(saveErrorMessage, err);
        return false;
    }

    // finally, encrypt everything
    setPhaseProgressBounds(SAVE_PROGRESS_ENCRYPTION);
    QByteArray encryptedData;
    err = encryptData(dataInBlocks, encryptedData);
    Util::safeClear(dataInBlocks);
    if (err != ErrorCodesV4::SUCCESS) {
        Util::safeClear(encryptedData);
        LOG("PwDatabaseV4::save() failed to encrypt data: %d", err);
        emit dbSaveError(saveErrorMessage, err);
        return false;
    }

    outStream.writeRawData(encryptedData.constData(), encryptedData.size());
    Util::safeClear(encryptedData);

    return true;
}

/**
 * Splits data to hashed blocks.
 */
ErrorCodesV4::ErrorCode PwDatabaseV4::splitToBlocks(const QByteArray& inData, QDataStream& blockStream) {
    static const int DEFAULT_BLOCK_SIZE = 1024 * 1024; // KeePass' default block size

    setPhaseProgressRawTarget(inData.size());

    CryptoManager* cm = CryptoManager::instance();
    QByteArray blockHash(SB_SHA256_DIGEST_LEN, 0);
    int blockStart = 0;
    quint32 blockSize;
    quint32 blockId = 0;
    QByteArray blockData;
    while (blockStart != inData.size()) {
        // write sequence: blockId, hash, size, block data;
        // end of data => zero size & all-zero hash

        blockSize = qMin(DEFAULT_BLOCK_SIZE, inData.size() - blockStart);

        blockData.setRawData(inData.data() + blockStart, blockSize); // reuses allocated memory, does not copy bytes
        if (cm->sha256(blockData, blockHash) != SB_SUCCESS)
            return ErrorCodesV4::BLOCK_HASHING_ERROR;

        writeBlock(blockStream, blockId, blockHash, blockSize, blockData);

        blockStart += blockSize;
        blockId++;
        increaseProgress(blockSize);
    }

    // finally, write the terminating block
    writeBlock(blockStream, blockId, QByteArray(SB_SHA256_DIGEST_LEN, 0), 0, QByteArray(/*empty*/));

    return ErrorCodesV4::SUCCESS;
}

// Helper for splitToBlocks()
void PwDatabaseV4::writeBlock(QDataStream& blockStream, quint32 blockId, const QByteArray& blockHash, quint32 blockSize, const QByteArray& blockData) {
    blockStream << blockId;
    blockStream.writeRawData(blockHash.constData(), blockHash.size());
    blockStream << blockSize;
    blockStream.writeRawData(blockData, blockSize);
}

/**
 * Encrypts DB's data using current keys.
 * Changes input raw data by adding padding.
 */
ErrorCodesV4::ErrorCode PwDatabaseV4::encryptData(QByteArray& rawData, QByteArray& encryptedData) {
    CryptoManager* cm = CryptoManager::instance();

    cm->addPadding16(rawData);
    int err = cm->encryptAES(SB_AES_CBC, aesKey, header.getInitialVector(), rawData, encryptedData, this);
    if (err != SB_SUCCESS) {
        LOG("encryptAES error: %d", err);
        return ErrorCodesV4::CANNOT_ENCRYPT_DB;
    }
    return ErrorCodesV4::SUCCESS;
}

/**
 * Changes DB's master key to the given combination.
 * Returns true if successful, otherwise emits an error and returns false.
 */
bool PwDatabaseV4::changeMasterKey(const QString& password, const QByteArray& keyFileData) {
    if (!PwDatabase::changeMasterKey(password, keyFileData))
        return false;

    meta.setMasterKeyChangedTime(QDateTime::currentDateTime());
    return true;
}

/**
 * Creates an DB v4 instance and initializes it with meaningful default values (header, meta, etc)
 * Also adds in a few sample groups/entries.
 * Set dbName to the database file name without extension.
 * Leaves master key uninitialized, so call changeMasterKey() after this method.
 */
PwDatabaseV4* PwDatabaseV4::createSampleDatabase(const QString& dbName) {
    PwDatabaseV4* db = new PwDatabaseV4();

    db->header.setCipherId(AES_ID); //uuid
    db->header.setCompressionFlags(PwHeaderV4::COMPRESSION_GZIP); //uint32
    db->header.setInnerRandomStreamId(SALSA_20_ID); // uuid
    db->header.setTransformRounds(DEFAULT_TRANSFORM_ROUNDS);

    // Master key is left non-initialized, as documented.
    db->header.randomizeInitialVectors();
    db->initSalsa20();

    // No need to set meta fields: they either have sensible default values,
    // or will be updated on save (e.g., hash and generator name)

    // Create the root and fill it with some groups/entries
    PwGroupV4* root = new PwGroupV4(db);
    db->_rootGroup = root;
    root->setUuid(PwUuid::create());
    root->setDatabase(db);
    root->setParentGroup(NULL);
    root->setIconId(49);
    root->setName(dbName);

    PwGroupV4* subGroupGeneral = dynamic_cast<PwGroupV4*>(root->createGroup());
    subGroupGeneral->setIconId(PwIcon::FOLDER);
    subGroupGeneral->setName(tr("General", "Name of a sample group intended for various/miscellaneous entries"));

    PwGroupV4* subGroupSystem = dynamic_cast<PwGroupV4*>(root->createGroup());
    subGroupSystem->setIconId(PwIcon::NETWORK_FOLDER_WINDOWS);
    subGroupSystem->setName(tr("System", "Name of a sample group intended for operating system related entries"));

    PwGroupV4* subGroupNetwork = dynamic_cast<PwGroupV4*>(root->createGroup());
    subGroupNetwork->setIconId(PwIcon::SERVER);
    subGroupNetwork->setName(tr("Network", "Name of a sample group intended for computer network related entries (servers, routers, etc)"));

    PwGroupV4* subGroupInternet = dynamic_cast<PwGroupV4*>(root->createGroup());
    subGroupInternet->setIconId(PwIcon::GLOBE);
    subGroupInternet->setName(tr("Internet", "Name of a sample group intended for general Internet-related entries (websites, forums, social networks, etc)"));

    PwGroupV4* subGroupEmail = dynamic_cast<PwGroupV4*>(root->createGroup());
    subGroupEmail->setIconId(PwIcon::ENVELOPE_OPEN);
    subGroupEmail->setName(tr("eMail", "Name of a sample group intended for e-mail related entries (email accounts)"));

    PwGroupV4* subGroupBanking = dynamic_cast<PwGroupV4*>(root->createGroup());
    subGroupBanking->setIconId(PwIcon::PERCENT);
    subGroupBanking->setName(tr("Homebanking", "Name of a sample group intended for banking/finance related entries (Internet banking, credit cards, payment gateways)"));

    PwEntryV4* entry1 = dynamic_cast<PwEntryV4*>(root->createEntry());
    entry1->setTitle(tr("Sample Entry", "Title of a sample entry"));
    entry1->setUserName(tr("john.smith", "Sample user name of a sample entry. Set to a typical person name for your language."));
    entry1->setPassword(tr("pa$$word", "Sample password of a sample entry. Translation is optional."));
    entry1->setUrl("http://keepassb.com");

    return db;
}
