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

// KeePass2 XML tag names
const QString XML_META = QString("Meta");
const QString XML_ROOT = QString("Root");
const QString XML_GROUP = QString("Group");
const QString XML_ENTRY = QString("Entry");

const QString XML_TITLE = QString("Title");
const QString XML_USERNAME = QString("UserName");
const QString XML_PASSWORD = QString("Password");
const QString XML_URL = QString("URL");
const QString XML_NOTES = QString("Notes");

const QString XML_UUID = QString("UUID");
const QString XML_NAME = QString("Name");
const QString XML_ICON_ID = QString("IconID");
const QString XML_STRING = QString("String");
const QString XML_HISTORY = QString("History");
const QString XML_KEY = QString("Key");
const QString XML_VALUE = QString("Value");
const QString XML_PROTECTED = QString("Protected");
const QString XML_TRUE = QString("True");
const QString XML_BINARY = QString("Binary");
const QString XML_BINARY_ID = QString("ID");
const QString XML_BINARY_COMPRESSED = QString("Compressed");
const QString XML_RECYCLE_BIN_UUID = QString("RecycleBinUUID");
const QString XML_REF = QString("Ref");

const QString XML_TIMES = QString("Times");
const QString XML_LAST_MODIFICATION_TIME = QString("LastModificationTime");
const QString XML_CREATION_TIME = QString("CreationTime");
const QString XML_LAST_ACCESS_TIME = QString("LastAccessTime");
const QString XML_EXPIRY_TIME = QString("ExpiryTime");

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

PwHeaderV4::ErrorCode PwHeaderV4::read(QDataStream& stream) {
    Q_ASSERT(!initialized);
    clear();

    // check file signatures (although probably checked before)
    quint32 sign1, sign2, fileVersion;
    stream >> sign1 >> sign2 >> fileVersion;
    size += 3 * sizeof(quint32);
    if (sign1 !=  SIGNATURE_1)
        return SIGNATURE_1_MISMATCH;
    if (sign2 != SIGNATURE_2)
        return SIGNATURE_2_MISMATCH;
    if ((fileVersion != 0x00030001) && (fileVersion != 0x00030000))
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
    return SUCCESS;
}

QString PwHeaderV4::getErrorMessage(ErrorCode errCode) {
    switch (errCode) {
    case SUCCESS:
        return "";
    case SIGNATURE_1_MISMATCH:
        // fallthrough
    case SIGNATURE_2_MISMATCH:
        return tr("Wrong file signature");
    case UNSUPPORTED_FILE_VERSION:
        return tr("Unsupported DB file version");
    case UNKNOWN_COMPRESSION_ALGORITHM:
        return tr("Unknown compression algorithm");
    case NOT_AES:
        return tr("Twofish cypher is not supported");
    default:
        return tr("Header error");
    }
}

void PwHeaderV4::clear() {
    data.clear();
    size = 0;
    transformRounds = 0;
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

int PwHeaderV4::sizeInBytes() const {
    return size;
}


/****************************/

PwDatabaseV4::PwDatabaseV4(QObject* parent) :
        PwDatabase(parent),
        header(),
        combinedKey(SB_SHA256_DIGEST_LEN, 0),
        aesKey(SB_SHA256_DIGEST_LEN, 0),
        salsa20(),
        binaries(),
        recycleBinGroupUuid() {
    header.setParent(this);
}

PwDatabaseV4::~PwDatabaseV4() {
    // nothing to do here
}

bool PwDatabaseV4::isSignatureMatch(const QByteArray& rawDbData) {
    QDataStream stream (rawDbData);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint32 sign1, sign2;
    stream >> sign1 >> sign2;

    return (sign1 ==  PwHeaderV4::SIGNATURE_1) && (sign2 == PwHeaderV4::SIGNATURE_2);
}

void PwDatabaseV4::clear() {
    qDeleteAll(binaries);
    binaries.clear();
    header.clear();
    combinedKey.clear();
    aesKey.clear();
    recycleBinGroupUuid.clear();
    PwDatabase::clear(); // ancestor's cleaning
}

void PwDatabaseV4::unlock(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) {
    clear();

    if (!buildCompositeKey(password.toUtf8(), keyFileData, combinedKey)) {
        emit dbUnlockError(tr("Crypto library error"), COMPOSITE_KEY_ERROR);
        return;
    }

    if (readDatabase(dbFileData))
        emit dbUnlocked();
}

bool PwDatabaseV4::buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const {
    CryptoManager* cm = CryptoManager::instance();

    QByteArray ba;
    int ec = cm->sha256(passwordKey, ba);
    if (ec != SB_SUCCESS)
        return false;

    // if no key file were supplied, the keyFileData will be empty
    QByteArray fKey;
    if (!keyFileData.isEmpty()) {
        if (!processKeyFile(keyFileData, fKey))
            return false;
        ba.append(fKey);
    }

    ec = cm->sha256(ba, combinedKey);
    if (ec != SB_SUCCESS)
        return false;

    return true;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::transformKey(const PwHeaderV4& header, const QByteArray& combinedKey, QByteArray& aesKey,
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
        return KEY_TRANSFORM_INIT_ERROR;

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
            emit unlockProgressChanged(progress);
        }
    }
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_1;

    if (cm->endKeyTransform() != SB_SUCCESS)
        return KEY_TRANSFORM_END_ERROR;


    QByteArray step2key;
    step2key.append(subKey1);
    step2key.append(subKey2);
    QByteArray transformedKey;
    ec = cm->sha256(step2key, transformedKey);
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_2;

    QByteArray step3key(header.getMasterSeed());
    step3key.append(transformedKey);
    ec = cm->sha256(step3key, aesKey);
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_3;

    return SUCCESS;
}

bool PwDatabaseV4::readDatabase(const QByteArray& dbBytes) {
    QDataStream stream (dbBytes);
    stream.setByteOrder(QDataStream::LittleEndian);

    emit unlockProgressChanged(UNLOCK_PROGRESS_INIT);

    PwHeaderV4::ErrorCode headerErrCode = header.read(stream);
    if (headerErrCode != PwHeaderV4::SUCCESS) {
        qDebug() << PwHeaderV4::getErrorMessage(headerErrCode) << headerErrCode;
        emit dbUnlockError(PwHeaderV4::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_HEADER_READ);

    /* Calculate the AES key */
    ErrorCode err = transformKey(header, combinedKey, aesKey, UNLOCK_PROGRESS_HEADER_READ, UNLOCK_PROGRESS_KEY_TRANSFORMED);
    if (err != SUCCESS) {
        qDebug() << "Cannot decrypt database - transformKey" << err;
        emit dbUnlockError(tr("Cannot decrypt database"), err);
        return false;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Decrypt data */
    int dataSize = dbBytes.size() - header.sizeInBytes();
    QByteArray decryptedData (dataSize, 0);
    // DB header not needed for decryption
    QByteArray dbBytesWithoutHeader = dbBytes.right(dataSize);
    err = decryptData(dbBytesWithoutHeader, decryptedData);
    if (err != SUCCESS) {
        qDebug() << "Cannot decrypt database - decryptData" << err;
        emit dbUnlockError(tr("Cannot decrypt database"), err);
        return false;
    }
    emit unlockProgressChanged(UNLOCK_PROGRESS_DECRYPTED);

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
    if (err != SUCCESS) {
        qDebug() << "Cannot decrypt database - readBlocks" << err;
        emit dbUnlockError(tr("Error reading database"), err);
        return false;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_BLOCKS_READ);

    QByteArray xmlData;
    if (header.isCompressed()) {
        /* Inflate GZip data to XML */
        Util::ErrorCode inflateErr = Util::inflateGZipData(blocksData, xmlData);
        if (inflateErr != Util::SUCCESS) {
            qDebug() << "Error inflating database";
            emit dbUnlockError(tr("Error inflating database"), inflateErr);
            return false;
        }
    } else {
        xmlData = blocksData;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_UNPACKED);

    /* Init Salsa20 for reading protected values */
    err = initSalsa20();
    if (err != SUCCESS) {
        qDebug() << "Cannot decrypt database - initSalsa20" << err;
        emit dbUnlockError(tr("Cannot decrypt database"), err);
        return false;
    }

    /* Parse XML */
    QString xmlString = QString::fromUtf8(xmlData.data(), xmlData.size());
    err = parseXml(xmlString);
    if (err != SUCCESS) {
        qDebug() << "Error parsing database" << err;
        emit dbUnlockError(tr("Error parsing database"), err);
        return false;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_DONE);
    qDebug() << "DB unlocked";

    return true;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::initSalsa20() {
    CryptoManager* cm = CryptoManager::instance();

    QByteArray salsaKey;
    int err = cm->sha256(header.getProtectedStreamKey(), salsaKey);
    if (err != SB_SUCCESS)
        return CANNOT_INIT_SALSA20;

    salsa20.init(salsaKey, SALSA_20_INIT_VECTOR);
    return SUCCESS;
}

/**
 * Decrypts the DB's data using current keys;
 * ReturnsKPB_* status code.
 */
PwDatabaseV4::ErrorCode PwDatabaseV4::decryptData(const QByteArray& encryptedData, QByteArray& decryptedData) {
    // assert encryptedData.size is multiple of 16 bytes

    CryptoManager* cm = CryptoManager::instance();
    int err = cm->decryptAES(aesKey, header.getInitialVector(), encryptedData, decryptedData);
    if (err != SB_SUCCESS) {
        qDebug() << "decryptAES error: " << err;
        return CANNOT_DECRYPT_DB;
    }
    return SUCCESS;
}

/**
 * Extracts data blocks from the decrypted data stream, verifying hashes.
 */
PwDatabaseV4::ErrorCode PwDatabaseV4::readBlocks(QDataStream& inputStream, QByteArray& blocksData) {
    quint32 blockSize, readBlockId;
    QByteArray blockHash(SB_SHA256_DIGEST_LEN, 0);
    QByteArray computedHash(SB_SHA256_DIGEST_LEN, 0);

    blocksData.clear();
    CryptoManager* cm = CryptoManager::instance();

    quint32 blockId = 0;
    while (true) {
        inputStream >> readBlockId;
        if (readBlockId != blockId) {
            qDebug() << "readBlocks wrong block ID";
            return WRONG_BLOCK_ID;
        }
        blockId++;

        inputStream.readRawData(blockHash.data(), SB_SHA256_DIGEST_LEN);
        inputStream >> blockSize;
        if (blockSize == 0) {
            if (Util::isAllZero(blockHash)) {
                break;
            } else {
                qDebug() << "readBlocks block hash is not all-zeros";
                return BLOCK_HASH_NON_ZERO;
            }
        }
        QByteArray blockData(blockSize, 0);
        inputStream.readRawData(blockData.data(), blockSize);
        int err = cm->sha256(blockData, computedHash);
        if ((err != SB_SUCCESS) || (computedHash != blockHash)) {
            qDebug() << "readBlocks block hash mismatch";
            return BLOCK_HASH_MISMATCH;
        }
        blocksData.append(blockData);
    }
    return SUCCESS;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::parseXml(const QString& xmlString) {
    if (_rootGroup) {
        delete _rootGroup;
        _rootGroup = NULL;
    }

    PwGroupV4* rootV4 = new PwGroupV4(this);
    rootV4->setParentGroup(NULL); // not Qt parent, but the group containing this one

    ErrorCode err;
    QXmlStreamReader xml(xmlString);
    QStringRef tagName;
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNextStartElement()) {
            tagName = xml.name();
            if (tagName == XML_META) {
                err = loadXmlMetaData(xml);
                if (err != SUCCESS)
                    return err;
            } else if (tagName == XML_ROOT) {
                if (xml.readNextStartElement() && (xml.name() == XML_GROUP)) {
                    err = loadGroupFromXml(xml, *rootV4);
                    if (err != SUCCESS)
                        return err;
                } else {
                    qDebug() << "SEVERE: there is no group in the root";
                    return XML_NO_ROOT_GROUP;
                }
            }
        }
    }
    xml.clear();
    if (xml.hasError())
        return XML_PARSING_ERROR;

    _rootGroup = rootV4;
//    debugPrint(_rootGroup, 2);
    return SUCCESS;
}

// loads metadata of a database
PwDatabaseV4::ErrorCode PwDatabaseV4::loadXmlMetaData(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_META);

    xml.readNext();
    QString tagName = xml.name().toString();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_META == tagName))) {
        if (xml.isStartElement()) {
            if (XML_RECYCLE_BIN_UUID == tagName) {
                QString recycleBinUuidBase64Str = xml.readElementText();
                recycleBinGroupUuid = PwUuid::fromBase64(recycleBinUuidBase64Str);
            } else if (tagName == XML_BINARY) {
                PwBinaryV4* binary = new PwBinaryV4();
                QXmlStreamAttributes attrs = xml.attributes();
                QString id = attrs.value(XML_BINARY_ID).toString();
                binary->isCompressed = (attrs.value(XML_BINARY_COMPRESSED) == XML_TRUE);
                binary->data = QByteArray::fromBase64(xml.readElementText().toLatin1());
                binaries.insert(id, binary);
            } else {
                // space for future extensions
            }
        }
        xml.readNext();
        tagName = xml.name().toString();
    }
    if (xml.hasError())
        return XML_PARSING_ERROR;
    else
        return SUCCESS;
}

// loads the group and its children
PwDatabaseV4::ErrorCode PwDatabaseV4::loadGroupFromXml(QXmlStreamReader& xml, PwGroupV4& group) {
    Q_ASSERT(xml.name() == XML_GROUP);

    ErrorCode err;
    xml.readNext();
    QString tagName = xml.name().toString();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_GROUP == tagName))) {
        if (xml.isStartElement()) {
            if (XML_UUID == tagName) {
                QString uuidBase64Str = xml.readElementText();
                PwUuid uuid = PwUuid::fromBase64(uuidBase64Str);
                group.setUuid(uuid);
                if (uuid == recycleBinGroupUuid)
                    group.setDeleted(true); // may also be set higher in call stack
            } else if (XML_ICON_ID == tagName) {
                QString iconIdStr = xml.readElementText();
                group.setIconId(iconIdStr.toInt(NULL));
            } else if (XML_NAME == tagName) {
                QString groupName = xml.readElementText();
                group.setName(groupName);
            } else if (XML_NOTES == tagName) {
                QString notes = xml.readElementText();
                group.setNotes(notes);
            } else if (XML_TIMES == tagName) {
                err = readGroupTimes(xml, group);
                if (err != SUCCESS)
                    return err;
            } else if (XML_GROUP == tagName) {
                PwGroupV4* subGroup = new PwGroupV4(&group);
                err = loadGroupFromXml(xml, *subGroup);
                if (err != SUCCESS) {
                    delete subGroup;
                    return err;
                }
                if (group.isDeleted())
                    subGroup->setDeleted(true); // propagate the deleted flag recursively
                group.addSubGroup(subGroup);
            } else if (XML_ENTRY == tagName) {
                PwEntryV4* entry = new PwEntryV4(&group);
                err = loadEntryFromXml(xml, *entry);
                if (err != SUCCESS) {
                    delete entry;
                    return err;
                }
                entry->setDeleted(group.isDeleted()); // propagate the deleted flag recursively
                group.addEntry(entry);
            }
        }
        xml.readNext();
        tagName = xml.name().toString();
    }

    if (xml.hasError())
        return XML_PARSING_ERROR;
    else
        return SUCCESS;
}

// loads the entry
PwDatabaseV4::ErrorCode PwDatabaseV4::loadEntryFromXml(QXmlStreamReader& xml, PwEntryV4& entry) {
    Q_ASSERT(xml.name() == XML_ENTRY);

    entry.clear();
    bool isConversionOk;
    ErrorCode err = SUCCESS;

    xml.readNext();
    QString tagName = xml.name().toString();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_ENTRY == tagName))) {
        if (xml.isStartElement()) {
            if (XML_UUID == tagName) {
                QString uuidBase64Str = xml.readElementText();
                entry.setUuid(PwUuid::fromBase64(uuidBase64Str));
            } else if (XML_ICON_ID == tagName) {
                int iconId = xml.readElementText().toInt(&isConversionOk, 10);
                if (isConversionOk)
                    entry.setIconId(iconId);
                else
                    return ICON_ID_IS_NOT_INTEGER;
            } else if (XML_STRING == tagName) {
                err = readEntryString(xml, entry);
            } else if (XML_BINARY == tagName) {
                PwAttachment* attachment = new PwAttachment(&entry);
                err = readEntryAttachment(xml, *attachment);
                entry.addAttachment(attachment);
            } else if (XML_TIMES == tagName) {
                err = readEntryTimes(xml, entry);
            } else if (XML_HISTORY == tagName) {
                err = readEntryHistory(xml, entry);
            }
        }
        if (err != SUCCESS)
            return err;
        xml.readNext();
        tagName = xml.name().toString();
    }
    if (xml.hasError())
        return XML_PARSING_ERROR;
    else
        return SUCCESS;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::readGroupTimes(QXmlStreamReader& xml, PwGroupV4& group) {
    Q_ASSERT(XML_TIMES == xml.name());

    QString text;
    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_TIMES))) {
        if (xml.isStartElement()) {
            if (tagName == XML_LAST_MODIFICATION_TIME) {
                text = xml.readElementText();
                group.setLastModificationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_CREATION_TIME) {
                text = xml.readElementText();
                group.setCreationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_LAST_ACCESS_TIME) {
                text = xml.readElementText();
                group.setLastAccessTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_EXPIRY_TIME) {
                text = xml.readElementText();
                group.setExpiryTime(QDateTime::fromString(text, Qt::ISODate));
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return XML_TIMES_PARSING_ERROR;
    else
        return SUCCESS;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::readEntryTimes(QXmlStreamReader& xml, PwEntryV4& entry) {
    Q_ASSERT(XML_TIMES == xml.name());

    QString text;
    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_TIMES))) {
        if (xml.isStartElement()) {
            if (tagName == XML_LAST_MODIFICATION_TIME) {
                text = xml.readElementText();
                entry.setLastModificationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_CREATION_TIME) {
                text = xml.readElementText();
                entry.setCreationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_LAST_ACCESS_TIME) {
                text = xml.readElementText();
                entry.setLastAccessTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_EXPIRY_TIME) {
                text = xml.readElementText();
                entry.setExpiryTime(QDateTime::fromString(text, Qt::ISODate));
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return XML_TIMES_PARSING_ERROR;
    else
        return SUCCESS;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::readEntryHistory(QXmlStreamReader& xml, PwEntryV4& hostEntry) {
    Q_ASSERT(XML_HISTORY == xml.name());

    ErrorCode err;
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_HISTORY))) {
        if (xml.isStartElement() && (tagName == XML_ENTRY)) {
            PwEntryV4* historyEntry = new PwEntryV4(&hostEntry); // hostEntry is a parent, not a copy source
            err = loadEntryFromXml(xml, *historyEntry);
            if (err != SUCCESS) {
                delete historyEntry;
                return err;
            }
            hostEntry.addHistoryEntry(historyEntry);
        }
        xml.readNext();
        tagName = xml.name();
    }
    return SUCCESS;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::readEntryString(QXmlStreamReader& xml, PwEntryV4& entry) {
    Q_ASSERT(XML_STRING == xml.name());

    QString key, value;
    QStringRef tagName = xml.name();

    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_STRING))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                key = xml.readElementText();
            } else if (tagName == XML_VALUE) {
                ErrorCode err = readEntryStringValue(xml, value);
                if (err != SUCCESS)
                    return err;
            }
        }
    }
    entry.setField(key, value);
    return SUCCESS;
}

// read a value from XML, decrypting it if necessary
PwDatabaseV4::ErrorCode PwDatabaseV4::readEntryStringValue(QXmlStreamReader& xml, QString& value) {
    Q_ASSERT(xml.name() == XML_VALUE);

    QXmlStreamAttributes attr = xml.attributes();

    if (attr.value(XML_PROTECTED) == XML_TRUE) {
        QString valueStr = xml.readElementText();
        QByteArray valueBytes = QByteArray::fromBase64(valueStr.toLatin1());
        int size = valueBytes.length();

        QByteArray salsaBytes;
        salsa20.getBytes(salsaBytes, size);

        const char* xorBuf = salsaBytes.constData();
        char* valueBuf = valueBytes.data();
        for (int i = 0; i < size; i++) {
            valueBuf[i] ^= xorBuf[i];
        }
        value = QString::fromUtf8(valueBuf, size);
    } else {
        value = xml.readElementText();
    }

    return SUCCESS;
}

PwDatabaseV4::ErrorCode PwDatabaseV4::readEntryAttachment(QXmlStreamReader& xml, PwAttachment& attachment) {
    Q_ASSERT(XML_BINARY == xml.name());

    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_BINARY))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                QString fileName = xml.readElementText();
                attachment.setName(fileName);
            } else if (tagName == XML_VALUE) {
                QString binaryRef = xml.attributes().value(XML_REF).toString();
                PwBinaryV4* binary = this->binaries.value(binaryRef);
                if (!binary) {
                    return INVALID_ATTACHMENT_REFERENCE;
                }
                attachment.setData(binary->data, binary->isCompressed);
            }
        }
    }
    return SUCCESS;
}

//TODO remove debug print
void PwDatabaseV4::debugPrint(const PwGroup* group, int indent) const {
    qDebug() << QString(indent*2, ' ') + group->toString();
    QList<PwEntry*> entries = group->getEntries();
    for (int i = 0; i < entries.size(); i++) {
        PwEntryV4* entry = reinterpret_cast<PwEntryV4*>(entries.at(i));
        qDebug() << QString((indent + 1)*2, ' ') + entry->toString();

//        QList<PwEntryV4*> historyEntries = entry->getHistoryEntries();
//        for (int j = 0; j < historyEntries.size(); j++)
//            qDebug() << QString((indent + 1)*2 + 1, ' ') + "(" + historyEntries.at(j)->toString() + ")";
    }
    QList<PwGroup*> subGroups = group->getSubGroups();
    for (int i = 0; i < subGroups.size(); i++) {
        debugPrint(subGroups[i], indent + 1);
    }
}
