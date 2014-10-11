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
// DB save stages progress percentage
const int SAVE_PROGRESS_INIT = 0;
const int SAVE_PROGRESS_CONTENT_PACKED = 5;
const int SAVE_PROGRESS_KEY_TRANSFORMED = 90;
const int SAVE_PROGRESS_DONE = 100;

const int MASTER_SEED_SIZE = 16;
const int INITIAL_VECTOR_SIZE = 16;
const int CONTENT_HASH_SIZE = 32;
const int TRANSFORM_SEED_SIZE = 32;

const QDateTime PwDatabaseV3::EXPIRY_DATE_NEVER = QDateTime(QDate(2999, 12, 28), QTime(23, 59, 59, 0), Qt::UTC);


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
        header(), combinedKey(), aesKey(), metaStreamEntries(), backupGroup(NULL) {
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
    backupGroup = NULL; // this is just a pointer, the actual group will be cleared by the base class
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

    emit progressChanged(UNLOCK_PROGRESS_INIT);

    PwHeaderV3::ErrorCode headerErrCode = header.read(stream);
    if (headerErrCode != PwHeaderV3::SUCCESS) {
        qDebug() << PwHeaderV3::getErrorMessage(headerErrCode) << headerErrCode;
        emit dbLoadError(PwHeaderV3::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }
    emit progressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Calculate the AES key */
    ErrorCode err = transformKey(combinedKey, aesKey, UNLOCK_PROGRESS_HEADER_READ, UNLOCK_PROGRESS_KEY_TRANSFORMED);
    if (err != SUCCESS) {
        qDebug() << "Cannot decrypt database - transformKey" << err;
        emit dbLoadError(tr("Cannot decrypt database", "A generic error message"), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

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
    emit progressChanged(UNLOCK_PROGRESS_DECRYPTED);

    QDataStream decryptedDataStream(decryptedData);
    decryptedDataStream.setByteOrder(QDataStream::LittleEndian);
    err = readContent(decryptedDataStream);
    if (err != SUCCESS) {
        emit dbLoadError(tr("Cannot parse database", "An error message. Parsing refers to the analysis/understanding of file content (do not confuse with reading it)."), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_DONE);

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
            emit progressChanged(progress);
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

PwDatabaseV3::ErrorCode PwDatabaseV3::readAllGroups(QDataStream& stream, QList<PwGroupV3*> &groups) {
    groups.clear();
    for (quint32 iGroup = 0; iGroup < header.getGroupCount(); iGroup++) {
        PwGroupV3* group = new PwGroupV3();
        group->setDatabase(this);
        if (!group->readFromStream(stream))
            return NOT_ENOUGH_GROUPS;
        if (group->isDeleted())
            backupGroup = group;
        groups.append(group);
    }
    return SUCCESS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readAllEntries(QDataStream& stream, QList<PwEntryV3*> &entries) {
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
    PwGroupV3* rootGroupV3 = new PwGroupV3();
    rootGroupV3->setDatabase(this);
    rootGroupV3->setLevel(-1); // because its children should have level 0
    // give the "virtual" root group some meaningful name
    rootGroupV3->setName(getDatabaseFileName());
    _rootGroup = rootGroupV3;
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
            PwGroupV3* group = groupById.value(groupId);
            entry->setDeleted(group->isDeleted());
            group->addEntry(entry);
        }
    }
    return SUCCESS;
}

/**
 * Encrypts and writes DB content to the given array.
 * In case of error emits a dbSaveError with error code and returns false.
 */
bool PwDatabaseV3::save(QByteArray& outData) {
    QString saveErrorMessage = tr("Cannot save database", "An error message");

    outData.clear();
    emit progressChanged(SAVE_PROGRESS_INIT);

    // pack up the groups&entries
    QByteArray contentData;
    int groupCount, entryCount;
    ErrorCode err = writeContent(contentData, groupCount, entryCount);
    if (err != SUCCESS) {
        emit dbSaveError(saveErrorMessage, err);
        return false;
    }
    emit progressChanged(SAVE_PROGRESS_CONTENT_PACKED);

    // now update the header (hash and counts)
    header.setGroupCount(groupCount);
    header.setEntryCount(entryCount);
    qDebug("Saving %d groups and %d entries", groupCount, entryCount);

    QByteArray contentHash;
    CryptoManager* cm = CryptoManager::instance();
    if (cm->sha256(contentData, contentHash) != SB_SUCCESS) {
        emit dbSaveError(saveErrorMessage, CONTENT_HASHING_ERROR);
        return false;
    }
    header.setContentHash(contentHash);

    // update encryption seeds
    header.randomizeInitialVectors();
    err = transformKey(combinedKey, aesKey, SAVE_PROGRESS_CONTENT_PACKED, SAVE_PROGRESS_KEY_TRANSFORMED);
    if (err != SUCCESS) {
        qDebug() << "transformKey error while saving: " << err;
        emit dbSaveError(saveErrorMessage, err);
        return false;
    }

    // write the header
    QDataStream outStream(&outData, QIODevice::WriteOnly);
    outStream.setByteOrder(QDataStream::LittleEndian);
    header.write(outStream);

    // encrypt the content
    cm->addPadding16(contentData);
    QByteArray encryptedContentData;
    cm->encryptAES(SB_AES_CBC, aesKey, header.getInitialVector(), contentData, encryptedContentData);
    Util::safeClear(contentData);

    emit progressChanged(SAVE_PROGRESS_DONE);

    outData.append(encryptedContentData);
    return true;
}

/**
 * Returns all the DB groups and entries
 */
void PwDatabaseV3::getAllChildren(QList<PwGroupV3*> &groups, QList<PwEntryV3*> &entries) {
    // The original KeePass 1 seems to store items in strange order: (normal groups, backup group, backup entries, normal entries).
    // We ignore this to save items in their original order.
    dynamic_cast<PwGroupV3*>(getRootGroup())->getAllChildren(groups, entries);
}

/**
 * Puts groups and entries data into the given array (without encryption).
 * Sets groupCount and entryCount to the number of saved groups/entries.
 */
PwDatabaseV3::ErrorCode PwDatabaseV3::writeContent(QByteArray& contentData, int& groupCount, int& entryCount) {
    QDataStream contentStream(&contentData, QIODevice::WriteOnly);
    contentStream.setByteOrder(QDataStream::LittleEndian);

    // first prepare the content
    QList<PwGroupV3*> groups;
    QList<PwEntryV3*> entries;
    getAllChildren(groups, entries);

    for (int i = 0; i < groups.size(); i++) {
        groups.at(i)->writeToStream(contentStream);
    }
    entries.append(metaStreamEntries);
    for (int i = 0; i < entries.size(); i++) {
        entries.at(i)->writeToStream(contentStream);
    }

    groupCount = groups.size();
    entryCount = entries.size();
    return SUCCESS;
}

/**
 * Generates a new group ID (guaranteeing it is not being used already)
 */
qint32 PwDatabaseV3::createNewGroupId() {
    QList<PwGroupV3*> groups;
    QList<PwEntryV3*> entries;
    getAllChildren(groups, entries);

    qint32 candidateId;
    bool isUnique = false;
    while (!isUnique) {
        candidateId = qrand();
        for (int i = 0; i < groups.size(); i++)
            if (candidateId == groups.at(i)->getId()) {
                continue;
            }
        isUnique = true;
    }
    groups.clear();
    entries.clear();

    return candidateId;
}

/**
 * Returns the Backup group of this database.
 * If createIfMissing is true, creates the group if it is missing.
 */
PwGroup* PwDatabaseV3::getBackupGroup(bool createIfMissing) {
    if (!backupGroup && createIfMissing) {
        // There's no backup group, let's make one
        PwGroupV3* rootGroupV3 = dynamic_cast<PwGroupV3*>(getRootGroup());
        backupGroup = dynamic_cast<PwGroupV3*>(rootGroupV3->createGroup());
        backupGroup->setName(PwGroupV3::BACKUP_GROUP_NAME);
        backupGroup->setIconId(PwGroupV3::BACKUP_GROUP_ICON_ID);
        backupGroup->setDeleted(TRUE);
    }
    return backupGroup;
}
