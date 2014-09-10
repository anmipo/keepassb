/*
 * PwDatabaseV3.cpp
 *
 *  Created on: 16 Aug 2014
 *      Author: Andrei
 */

#include "PwDatabaseV3.h"
#include <QObject>
#include <QDataStream>
#include "sbreturn.h"
#include "husha2.h"
#include "huaes.h"
#include "db/PwUuid.h"
#include "util/Util.h"

// DB unlock stages progress percentage
const int UNLOCK_PROGRESS_INIT = 0;
const int UNLOCK_PROGRESS_HEADER_READ = 5;
const int UNLOCK_PROGRESS_KEY_TRANSFORMED = 70;
const int UNLOCK_PROGRESS_DECRYPTED = 80;
const int UNLOCK_PROGRESS_DONE = 100;

const int MASTER_SEED_SIZE = 16;
const int INITIAL_VECTOR_SIZE = 16;
const int CONTENT_HASH_SIZE = 32;
const int TRANSFORM_SEED_SIZE = 32;

PwHeaderV3::PwHeaderV3(QObject* parent) : QObject(parent),
        masterSeed(), initialVector(), contentHash(), transformSeed() {
    flags = 0;
    transformRounds = 0;
    groupCount = 0;
    entryCount = 0;
}

PwHeaderV3::~PwHeaderV3() {
    clear();
}

void PwHeaderV3::clear() {
    flags = 0;
    Util::safeClear(masterSeed);
    Util::safeClear(initialVector);
    Util::safeClear(contentHash);
    Util::safeClear(transformSeed);
    transformRounds = 0;
    groupCount = 0;
    entryCount = 0;
}

PwHeaderV3::ErrorCode PwHeaderV3::read(QDataStream& stream) {
    clear();

    // check file signatures (although probably checked before)
    quint32 sign1, sign2, fileVersion;
    stream >> sign1 >> sign2 >> flags >> fileVersion;
    if (sign1 !=  SIGNATURE_1)
        return SIGNATURE_1_MISMATCH;
    if (sign2 != SIGNATURE_2)
        return SIGNATURE_2_MISMATCH;
    if ((flags & FLAG_TWOFISH) || !(flags & FLAG_RIJNDAEL))
        return NOT_AES;
    if ((fileVersion & 0xFFFFFF00) != (DB_VERSION & 0xFFFFFF00))
        return UNSUPPORTED_FILE_VERSION;

    qDebug("Signatures match");

    masterSeed.fill(0, MASTER_SEED_SIZE);
    stream.readRawData(masterSeed.data(), MASTER_SEED_SIZE);
    initialVector.fill(0, INITIAL_VECTOR_SIZE);
    stream.readRawData(initialVector.data(), INITIAL_VECTOR_SIZE);

    stream >> groupCount >> entryCount;

    contentHash.fill(0, CONTENT_HASH_SIZE);
    stream.readRawData(contentHash.data(), CONTENT_HASH_SIZE);
    transformSeed.fill(0, TRANSFORM_SEED_SIZE);
    stream.readRawData(transformSeed.data(), TRANSFORM_SEED_SIZE);

    stream >> transformRounds;
    return SUCCESS;
}

/** Resets encryption IV, master and transform seeds to (securely) random values. */
PwHeaderV3::ErrorCode PwHeaderV3::randomizeInitialVectors() {
    CryptoManager* cm = CryptoManager::instance();
    int err = cm->getRandomBytes(initialVector, INITIAL_VECTOR_SIZE);
    if (err != SB_SUCCESS)
        return ERROR_RANDOMIZING_IVS;
    err = cm->getRandomBytes(masterSeed, MASTER_SEED_SIZE);
    if (err != SB_SUCCESS)
        return ERROR_RANDOMIZING_IVS;
    err = cm->getRandomBytes(transformSeed, TRANSFORM_SEED_SIZE);
    if (err != SB_SUCCESS)
        return ERROR_RANDOMIZING_IVS;
    return SUCCESS;
}

/** Erases loaded data from memory */
PwHeaderV3::ErrorCode PwHeaderV3::write(QDataStream& outStream) {
    outStream << SIGNATURE_1 << SIGNATURE_2 << flags << DB_VERSION;
    outStream.writeRawData(masterSeed.constData(), masterSeed.size());
    outStream.writeRawData(initialVector.constData(), initialVector.size());
    outStream << groupCount << entryCount;
    outStream.writeRawData(contentHash.constData(), contentHash.size());
    outStream.writeRawData(transformSeed.constData(), transformSeed.size());
    outStream << transformRounds;
    return SUCCESS;
}

QString PwHeaderV3::getErrorMessage(ErrorCode errCode) {
    switch (errCode) {
    case SUCCESS:
        return "";
    case SIGNATURE_1_MISMATCH:
        // fallthrough
    case SIGNATURE_2_MISMATCH:
        return tr("Wrong database signature", "Error message when opening a database.");
    case UNSUPPORTED_FILE_VERSION:
        return tr("Unsupported database version", "Error message when opening a database.");
    case NOT_AES:
        return tr("Twofish cypher is not supported", "Error message when opening a database. 'Twofish' is an algorithm name, do not translate it.");
    default:
        return tr("Header error", "Error message when opening a database. 'Header' refers to supplemental data placed at the beginning of a file.");
    }
}


PwDatabaseV3::PwDatabaseV3(QObject* parent) : PwDatabase(parent),
        header(), combinedKey(), aesKey(), metaStreamEntries() {
    header.setParent(this);
}

PwDatabaseV3::~PwDatabaseV3() {
    clear();
}

void PwDatabaseV3::clear() {
    qDeleteAll(metaStreamEntries);
    metaStreamEntries.clear();
    header.clear();
    Util::safeClear(combinedKey);
    Util::safeClear(aesKey);
    PwDatabase::clear();
}

bool PwDatabaseV3::isSignatureMatch(const QByteArray& rawDbData) {
    QDataStream stream(rawDbData);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint32 sign1, sign2;
    stream >> sign1 >> sign2;

    return (sign1 ==  PwHeaderV3::SIGNATURE_1) && (sign2 == PwHeaderV3::SIGNATURE_2);
}

void PwDatabaseV3::load(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) {
    if (!buildCompositeKey(password.toLatin1(), keyFileData, combinedKey)) {
        emit dbLoadError(tr("Cryptographic library error", "Generic error message from a cryptographic library"), COMPOSITE_KEY_ERROR);
        return;
    }

    if (readDatabase(dbFileData)) {
        emit dbUnlocked();
    }
}

bool PwDatabaseV3::buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const {
    CryptoManager* cm = CryptoManager::instance();

    int err;
    // if no key file were supplied, the keyFileData will be empty
    if (keyFileData.isEmpty()) {
        err = cm->sha256(passwordKey, combinedKey);
    } else {
        QByteArray passwordHash, fKey;
        err = cm->sha256(passwordKey, passwordHash);
        if (err != SB_SUCCESS)
            return false;

        if (!processKeyFile(keyFileData, fKey))
            return false;
        passwordHash.append(fKey);

        err = cm->sha256(passwordHash, combinedKey);
    }
    return (err == SB_SUCCESS);
}

bool PwDatabaseV3::readDatabase(const QByteArray& dbBytes) {
    QDataStream stream (dbBytes);
    stream.setByteOrder(QDataStream::LittleEndian);

    emit unlockProgressChanged(UNLOCK_PROGRESS_INIT);

    PwHeaderV3::ErrorCode headerErrCode = header.read(stream);
    if (headerErrCode != PwHeaderV3::SUCCESS) {
        qDebug() << PwHeaderV3::getErrorMessage(headerErrCode) << headerErrCode;
        emit dbLoadError(PwHeaderV3::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }
    emit unlockProgressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Calculate the AES key */
    ErrorCode err = transformKey(combinedKey, aesKey, UNLOCK_PROGRESS_HEADER_READ, UNLOCK_PROGRESS_KEY_TRANSFORMED);
    if (err != SUCCESS) {
        qDebug() << "Cannot decrypt database - transformKey" << err;
        emit dbLoadError(tr("Cannot decrypt database", "A generic error message"), err);
        return false;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Decrypt data */
    int dataSize = dbBytes.size() - header.HEADER_SIZE;
    // DB header not needed for decryption
    QByteArray dbBytesWithoutHeader = dbBytes.right(dataSize);
    QByteArray decryptedData(dataSize, 0);
    err = decryptData(dbBytesWithoutHeader, decryptedData);
    if (err != SUCCESS) {
        if (err == DECRYPTED_PADDING_ERROR || err == DECRYPTED_CHECKSUM_MISMATCH) {
            qDebug() << "Cannot decrypt database - decryptData" << err;
            emit invalidPasswordOrKey();
        } else {
            // err == CANNOT_DECRYPT_DB
            // err == CONTENT_HASHING_ERROR
            // err == something else
            qDebug() << "Cannot decrypt database - decryptData" << err;
            emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        }
        return false;
    }
    emit unlockProgressChanged(UNLOCK_PROGRESS_DECRYPTED);

    QDataStream decryptedDataStream(decryptedData);
    decryptedDataStream.setByteOrder(QDataStream::LittleEndian);
    err = readContent(decryptedDataStream);
    if (err != SUCCESS) {
        emit dbLoadError(tr("Cannot parse database", "An error message. Parsing refers to the analysis/understanding of file content (do not confuse with reading it)."), err);
        return false;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_DONE);

    return true;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::transformKey(const QByteArray& combinedKey,
        QByteArray& aesKey, const int progressFrom, const int progressTo) {
    CryptoManager* cm = CryptoManager::instance();

    // prepare key transform
    if (cm->beginKeyTransform(header.getTransformSeed(), SB_AES_256_KEY_BYTES) != SB_SUCCESS)
        return KEY_TRANSFORM_INIT_ERROR;

    quint32 transformRounds = header.getTransformRounds();
    int progress = progressFrom;
    int subProgress = 0;
    int subProgressThreshold = ceil(transformRounds / (progressTo - progressFrom));
    int ec;

    QByteArray transformedKey(SB_AES_256_KEY_BYTES, 0);
    QByteArray combinedKey2(combinedKey.data(), combinedKey.size()); // this makes a deep copy
    unsigned char* origKey = reinterpret_cast<unsigned char*>(combinedKey2.data());
    unsigned char* transKey = reinterpret_cast<unsigned char*>(transformedKey.data());

    for (quint64 round = 0; round < transformRounds; round++) {
        ec = cm->performKeyTransform(origKey, transKey);
        memcpy(origKey, transKey, SB_AES_256_KEY_BYTES);
        if (ec != SB_SUCCESS) break;

        if (++subProgress > subProgressThreshold) {
            subProgress = 0;
            progress++;
            emit unlockProgressChanged(progress);
        }
    }
    Util::safeClear(combinedKey2); // ~ origKey
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_1;

    // clean up
    if (cm->endKeyTransform() != SB_SUCCESS)
        return KEY_TRANSFORM_END_ERROR;

    QByteArray prefinalKey;
    ec = cm->sha256(transformedKey, prefinalKey);
    Util::safeClear(transformedKey);
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_2;

    prefinalKey.prepend(header.getMasterSeed());
    ec = cm->sha256(prefinalKey, aesKey);
    Util::safeClear(prefinalKey);
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_3;

    return SUCCESS;
}

// Decrypts the DB's data using current keys.
PwDatabaseV3::ErrorCode PwDatabaseV3::decryptData(const QByteArray& encryptedData, QByteArray& decryptedData) {
    CryptoManager* cm = CryptoManager::instance();
    int err = cm->decryptAES(aesKey, header.getInitialVector(), encryptedData, decryptedData);
    if (err != SB_SUCCESS) {
        qDebug() << "decryptAES error: " << err;
        return CANNOT_DECRYPT_DB;
    }

    if (!CryptoManager::removePadding16(decryptedData))
        return DECRYPTED_PADDING_ERROR;

    QByteArray decryptedContentHash;
    err = cm->sha256(decryptedData, decryptedContentHash);
    if (err != SB_SUCCESS) {
        qDebug() << "Failed to hash decrypted data" << err;
        return CONTENT_HASHING_ERROR;
    }

    if (decryptedContentHash != header.getContentHash()) {
        qDebug() << "Decrypted checksum mismatch";
        return DECRYPTED_CHECKSUM_MISMATCH;
    }
    qDebug() << "Checksum matches";

    return SUCCESS;
}

/** Reads a 5-byte V3-specific timestamp from the stream */
QDateTime PwDatabaseV3::readTimestamp(QDataStream& stream) {
    quint8 dw1, dw2, dw3, dw4, dw5;
    stream >> dw1 >> dw2 >> dw3 >> dw4 >> dw5; // 31, 122, 33, 42, 210

    int year = (dw1 << 6) | (dw2 >> 2); // 2014
    int month = ((dw2 & 0x00000003) << 2) | (dw3 >> 6); // 8
    int day = (dw3 >> 1) & 0x0000001F; // 16
    QDate date(year, month, day);

    int hour = ((dw3 & 0x00000001) << 4) | (dw4 >> 4); // 18
    int minute = ((dw4 & 0x0000000F) << 2) | (dw5 >> 6); // 43
    int second = dw5 & 0x0000003F; // 18
    QTime time(hour, minute, second, 0);

    QDateTime result(date, time, Qt::UTC);
    return result;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readAllGroups(QDataStream& stream, QList<PwGroupV3*> &groups) {
    PwDatabaseV3::ErrorCode err;
    groups.clear();
    for (quint32 iGroup = 0; iGroup < header.getGroupCount(); iGroup++) {
        PwGroupV3* group = new PwGroupV3();
        if (!group->readFromStream(stream))
            return NOT_ENOUGH_GROUPS;
        groups.append(group);
    }
    return SUCCESS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readAllEntries(QDataStream& stream, QList<PwEntryV3*> &entries) {
    PwDatabaseV3::ErrorCode err;
    for (quint32 iEntry = 0; iEntry < header.getEntryCount(); iEntry++) {
        PwEntryV3* entry = new PwEntryV3();
        if (!entry->readFromStream(stream))
            return NOT_ENOUGH_ENTRIES;
        entries.append(entry);
    }
    return SUCCESS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readContent(QDataStream& stream) {
    QList<PwGroupV3*> groups;
    ErrorCode err = readAllGroups(stream, groups);
    if (err != SUCCESS)
        return err;

    QList<PwEntryV3*> entries;
    err = readAllEntries(stream, entries);
    if (err != SUCCESS)
        return err;

    // make a group-by-ID lookup hashtable;
    // also find the maximum group level
    int maxLevel = 0;
    quint16 level;
    PwGroupV3* group;
    QHash<qint32, PwGroupV3*> groupById;
    for (int i = 0; i < groups.size(); i++) {
        group = groups.at(i);
        groupById.insert(group->getId(), group);
        level = group->getLevel();
        if (level > maxLevel)
            maxLevel = level;
    }

    // restore group hierarchy
    _rootGroup = new PwGroupV3();
    // give the "virtual" root group some meaningful name
    _rootGroup->setName(getDatabaseFileName());
    PwGroupV3* parentGroup = (PwGroupV3*)_rootGroup;
    for (quint16 level = 0; level <= maxLevel; level++) {
        quint16 prevLevel = level - 1;
        for (int i = 0; i < groups.size(); i++) {
            PwGroupV3* group = groups.at(i);
            quint16 curLevel = group->getLevel();
            if (curLevel == level) {
                parentGroup->addSubGroup(group);
            } else if (curLevel == prevLevel) {
                parentGroup = group;
            }
        }
    }

    // put entries to their groups
    PwEntryV3* entry;
    for (int i = 0; i < entries.size(); i++) {
        entry = entries.at(i);
        if (entry->isMetaStream()) {
            // meta streams are kept in their own list, invisibly for the user
            metaStreamEntries.append(entry);
        } else {
            qint32 groupId = entry->getGroupId();
            if (!groupById.contains(groupId)) {
                qDebug() << "There is an entry " << entry->toString() << " with unknown groupId: " << groupId;
                return ORPHANED_ENTRY_ERROR;
            }
            groupById.value(groupId)->addEntry(entry);
        }
    }
    return SUCCESS;
}

/**
 * Encrypts and writes DB content to the given array.
 */
void PwDatabaseV3::save(QByteArray& outData) {
    // TODO implement V3 saving
    outData.clear();
    QDataStream outStream(&outData, QIODevice::WriteOnly);
    outStream.setByteOrder(QDataStream::LittleEndian);

//    header.randomizeInitialVectors(); // TOOD uncomment after debug
    header.write(outStream);
}
