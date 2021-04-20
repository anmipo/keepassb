/*
 * PwDatabaseV3.cpp
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
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
// DB unlock stages progress percentage
const quint8 UNLOCK_PROGRESS_KEY_TRANSFORM[]  = { 0, 70};
const quint8 UNLOCK_PROGRESS_DECRYPTION[]     = {70, 80};
const quint8 UNLOCK_PROGRESS_PARSE_DATA[]     = {80, 100};

// DB save stages progress percentage
const quint8 SAVE_PROGRESS_PACK_DATA[]      = { 0,  5};
const quint8 SAVE_PROGRESS_KEY_TRANSFORM[]  = { 5, 90};
const quint8 SAVE_PROGRESS_ENCRYPTION[]     = {90, 100};

const int MASTER_SEED_SIZE = 16;
const int INITIAL_VECTOR_SIZE = 16;
const int CONTENT_HASH_SIZE = 32;
const int TRANSFORM_SEED_SIZE = 32;

const QDateTime PwDatabaseV3::EXPIRY_DATE_NEVER = QDateTime(QDate(2999, 12, 28), QTime(23, 59, 59, 0), Qt::LocalTime);


PwHeaderV3::PwHeaderV3(QObject* parent) : QObject(parent),
        masterSeed(), initialVector(), contentHash(), transformSeed() {
    clear();
}

PwHeaderV3::~PwHeaderV3() {
    clear();
}

void PwHeaderV3::clear() {
    flags = 0;
    cypherAlgorithm = ALGORITHM_AES;
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
    if ((fileVersion & 0xFFFFFF00) != (DB_VERSION & 0xFFFFFF00))
        return UNSUPPORTED_FILE_VERSION;

    if ((flags & FLAG_RIJNDAEL) || !(flags & FLAG_TWOFISH)) {
        cypherAlgorithm = ALGORITHM_AES;
    } else if ((flags & FLAG_TWOFISH) || !(flags & FLAG_RIJNDAEL)) {
        cypherAlgorithm = ALGORITHM_TWOFISH;
    } else
        return UNSUPPORTED_CYPHER;

    LOG("Signatures match");

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
    case UNSUPPORTED_CYPHER:
        return tr("Unsupported cypher type", "Error message when opening a database.");
    default:
        return tr("Header error %1", "Error message when opening a database. 'Header' refers to supplemental data placed at the beginning of a file.").arg(errCode);
    }
}


PwDatabaseV3::PwDatabaseV3(QObject* parent) : PwDatabase(parent),
        header(), combinedKey(), masterKey(), metaStreamEntries(), backupGroup(NULL) {
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
    Util::safeClear(masterKey);
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
QByteArray PwDatabaseV3::getPasswordBytes(const QString& password) const {
    return password.toLatin1();
}

/** Setter for the combinedKey field */
void PwDatabaseV3::setCombinedKey(const QByteArray& newKey) {
    combinedKey = Util::deepCopy(newKey);
}

/**
 * Extracts the key from a correctly-formed XML file.
 * Returns true if successful, false otherwise.
 */
bool PwDatabaseV3::processXmlKeyFile(const QByteArray& keyFileData, QByteArray& key) const {
    Q_UNUSED(keyFileData); Q_UNUSED(key);
    // By design, V3 does not handle XML key files in any special manner.
    return false;
}

bool PwDatabaseV3::buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const {
    CryptoManager* cm = CryptoManager::instance();
    int err;
    // if no key file were supplied, the keyFileData will be empty
    if (!passwordKey.isEmpty() && !keyFileData.isEmpty()) {
        LOG("using password and key file");
        QByteArray passwordHash, fKey;
        err = cm->sha256(passwordKey, passwordHash);
        if (err != SB_SUCCESS)
            return false;

        if (!processKeyFile(keyFileData, fKey))
            return false;
        passwordHash.append(fKey);

        err = cm->sha256(passwordHash, combinedKey);
        return (err == SB_SUCCESS);
    } else if (keyFileData.isEmpty()) {
        LOG("using password only");
        err = cm->sha256(passwordKey, combinedKey);
        return (err == SB_SUCCESS);
    } else if (passwordKey.isEmpty()) {
        LOG("using key file only");
        return processKeyFile(keyFileData, combinedKey);
    } else {
        LOG("empty keys provided (should not happen)");
        return false;
    }
}

bool PwDatabaseV3::readDatabase(const QByteArray& dbBytes) {
    QDataStream stream (dbBytes);
    stream.setByteOrder(QDataStream::LittleEndian);

    PwHeaderV3::ErrorCode headerErrCode = header.read(stream);
    if (headerErrCode != PwHeaderV3::SUCCESS) {
        LOG("%s: %d", PwHeaderV3::getErrorMessage(headerErrCode).toUtf8().constData(), headerErrCode);
        emit dbLoadError(PwHeaderV3::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }

    /* Calculate the encryption key */
    setPhaseProgressBounds(UNLOCK_PROGRESS_KEY_TRANSFORM);
    PwDatabase::ErrorCode dbErr = transformKey(header.getMasterSeed(), header.getTransformSeed(),
            header.getTransformRounds(), combinedKey, masterKey);
    if (dbErr != PwDatabase::SUCCESS) {
        LOG("Cannot decrypt database - transformKey: %d", dbErr);
        emit dbLoadError(tr("Cannot decrypt database", "A generic error message"), dbErr);
        return false;
    }


    /* Decrypt data */
    setPhaseProgressBounds(UNLOCK_PROGRESS_DECRYPTION);
    int dataSize = dbBytes.size() - header.HEADER_SIZE;
    // DB header not needed for decryption
    QByteArray dbBytesWithoutHeader = dbBytes.right(dataSize);
    QByteArray decryptedData(dataSize, 0);
    ErrorCode err = decryptData(dbBytesWithoutHeader, decryptedData);
    Util::safeClear(dbBytesWithoutHeader);
    if (err != SUCCESS) {
        if (err == DECRYPTED_PADDING_ERROR || err == DECRYPTED_CHECKSUM_MISMATCH) {
            LOG("Cannot decrypt database - decryptData: %d", err);
            emit invalidPasswordOrKey();
        } else {
            // err == CANNOT_DECRYPT_DB
            // err == CONTENT_HASHING_ERROR
            // err == something else
            LOG("Cannot decrypt database - decryptData: %d", err);
            emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        }
        return false;
    }

    /* Reading and parsing data*/
    setPhaseProgressBounds(UNLOCK_PROGRESS_PARSE_DATA);
    QDataStream decryptedDataStream(decryptedData);
    decryptedDataStream.setByteOrder(QDataStream::LittleEndian);
    err = readContent(decryptedDataStream);
    Util::safeClear(decryptedData);
    if (err != SUCCESS) {
        emit dbLoadError(tr("Cannot parse database", "An error message. Parsing refers to the analysis/understanding of file content (do not confuse with reading it)."), err);
        return false;
    }

    return true;
}

// Decrypts the DB's data using current keys.
PwDatabaseV3::ErrorCode PwDatabaseV3::decryptData(const QByteArray& encryptedData, QByteArray& decryptedData) {
    CryptoManager* cm = CryptoManager::instance();
    int err;
    switch (header.getCypherAlgorithm()) {
    case PwHeaderV3::ALGORITHM_AES:
        err = cm->decryptAES(masterKey, header.getInitialVector(), encryptedData, decryptedData, this);
        if (err != SB_SUCCESS) {
            LOG("decryptAES error: %d", err);
            return CANNOT_DECRYPT_DB_AES;
        }
        break;
    case PwHeaderV3::ALGORITHM_TWOFISH:
        err = cm->decryptTwofish(masterKey, header.getInitialVector(), encryptedData, decryptedData, this);
        if (err != SB_SUCCESS) {
            LOG("decryptTwofish error: %d", err);
            return CANNOT_DECRYPT_DB_TWOFISH;
        }
        break;
    default:
        LOG("This should not happen: Unexpected cypher algorithm");
        return CANNOT_DECRYPT_DB;
    }

    if (!CryptoManager::removePadding16(decryptedData))
        return DECRYPTED_PADDING_ERROR;

    QByteArray decryptedContentHash;
    err = cm->sha256(decryptedData, decryptedContentHash);
    if (err != SB_SUCCESS) {
        LOG("Failed to hash decrypted data: %d", err);
        return CONTENT_HASHING_ERROR;
    }

    if (decryptedContentHash != header.getContentHash()) {
        LOG("Decrypted checksum mismatch");
        return DECRYPTED_CHECKSUM_MISMATCH;
    }
    LOG("Checksum matches");

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
        increaseProgress(1);
    }
    return SUCCESS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readAllEntries(QDataStream& stream, QList<PwEntryV3*> &entries) {
    for (quint32 iEntry = 0; iEntry < header.getEntryCount(); iEntry++) {
        PwEntryV3* entry = new PwEntryV3();
        if (!entry->readFromStream(stream))
            return NOT_ENOUGH_ENTRIES;
        entries.append(entry);
        increaseProgress(1);
    }
    return SUCCESS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readContent(QDataStream& stream) {

    setPhaseProgressRawTarget(header.getGroupCount() + header.getEntryCount());

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
    rootGroupV3->setIconId(PwGroup::DEFAULT_ICON_ID); // created subgroups will use this icon
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
                LOG("There is an entry %s with unknown groupId: %d", entry->toString().toUtf8().constData(), groupId);
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

    // pack up the groups&entries
    setPhaseProgressBounds(SAVE_PROGRESS_PACK_DATA);
    QByteArray contentData;
    int groupCount, entryCount;
    ErrorCode err = writeContent(contentData, groupCount, entryCount);
    if (err != SUCCESS) {
        emit dbSaveError(saveErrorMessage, err);
        return false;
    }

    // now update the header (hash and counts)
    header.setGroupCount(groupCount);
    header.setEntryCount(entryCount + metaStreamEntries.size());
    LOG("Saving %d groups and %d entries", groupCount, entryCount);

    QByteArray contentHash;
    CryptoManager* cm = CryptoManager::instance();
    if (cm->sha256(contentData, contentHash) != SB_SUCCESS) {
        emit dbSaveError(saveErrorMessage, CONTENT_HASHING_ERROR);
        return false;
    }
    header.setContentHash(contentHash);

    // update encryption seeds and transform the keys
    header.randomizeInitialVectors();
    setPhaseProgressBounds(SAVE_PROGRESS_KEY_TRANSFORM);
    PwDatabase::ErrorCode dbErr = transformKey(header.getMasterSeed(), header.getTransformSeed(),
            header.getTransformRounds(), combinedKey, masterKey);
    if (dbErr != PwDatabase::SUCCESS) {
        LOG("transformKey error while saving: %d", dbErr);
        emit dbSaveError(saveErrorMessage, dbErr);
        return false;
    }

    // write the header
    QDataStream outStream(&outData, QIODevice::WriteOnly);
    outStream.setByteOrder(QDataStream::LittleEndian);
    header.write(outStream);

    // encrypt the content
    setPhaseProgressBounds(SAVE_PROGRESS_ENCRYPTION);
    QByteArray encryptedContentData;
    err = encryptContent(contentData, encryptedContentData);
    Util::safeClear(contentData);
    if (err != SUCCESS) {
        LOG("encryptContent error while saving: %d", err);
        emit dbSaveError(saveErrorMessage, err);
        return false;
    }
    outData.append(encryptedContentData);
    return true;
}

/**
 * Encrypts content data using current keys.
 * May change contentData by padding it.
 */
PwDatabaseV3::ErrorCode PwDatabaseV3::encryptContent(QByteArray& contentData, QByteArray& encryptedContentData) {
    CryptoManager* cm = CryptoManager::instance();
    cm->addPadding16(contentData);

    int err;
    switch (header.getCypherAlgorithm()) {
    case PwHeaderV3::ALGORITHM_AES:
        err = cm->encryptAES(SB_AES_CBC, masterKey, header.getInitialVector(), contentData, encryptedContentData, this);
        if (err != SB_SUCCESS) {
            LOG("encryptAES error while saving: %d", err);
            return CANNOT_ENCRYPT_DB_AES;
        }
        break;
    case PwHeaderV3::ALGORITHM_TWOFISH:
        err = cm->encryptTwofish(masterKey, header.getInitialVector(), contentData, encryptedContentData, this);
        if (err != SB_SUCCESS) {
            LOG("encryptTwofish error while saving: %d", err);
            return CANNOT_ENCRYPT_DB_TWOFISH;
        }
        break;
    default:
        LOG("This should not happen: Unexpected cypher algorithm");
        return CANNOT_ENCRYPT_DB;
    }
    return SUCCESS;
}

/**
 * Puts groups and entries data into the given array (without encryption).
 * Sets groupCount and entryCount to the number of saved groups/entries.
 */
PwDatabaseV3::ErrorCode PwDatabaseV3::writeContent(QByteArray& contentData, int& groupCount, int& entryCount) {
    QDataStream contentStream(&contentData, QIODevice::WriteOnly);
    contentStream.setByteOrder(QDataStream::LittleEndian);

    setPhaseProgressRawTarget((quint64)(groupCount + entryCount));

    // first prepare the content
    QList<PwGroup*> groups;
    QList<PwEntry*> entries;
    getRootGroup()->getAllChildren(groups, entries);

    for (int i = 0; i < groups.size(); i++) {
        dynamic_cast<PwGroupV3*>(groups.at(i))->writeToStream(contentStream);
        increaseProgress(1);
    }

    for (int i = 0; i < entries.size(); i++) {
        dynamic_cast<PwEntryV3*>(entries.at(i))->writeToStream(contentStream);
        increaseProgress(1);
    }
    // also write the meta-stream entries (which are not included in the above list)
    for (int i = 0; i < metaStreamEntries.size(); i++) {
        metaStreamEntries.at(i)->writeToStream(contentStream);
    }

    groupCount = groups.size();
    entryCount = entries.size();

    groups.clear();
    entries.clear();

    return SUCCESS;
}

/**
 * Generates a new group ID (guaranteeing it is not being used already)
 */
qint32 PwDatabaseV3::createNewGroupId() {
    QList<PwGroup*> groups;
    QList<PwEntry*> entries;
    getRootGroup()->getAllChildren(groups, entries);

    qint32 candidateId;
    PwGroupV3* groupV3;
    bool isUnique = false;
    while (!isUnique) {
        candidateId = qrand();
        for (int i = 0; i < groups.size(); i++) {
            groupV3 = dynamic_cast<PwGroupV3*>(groups.at(i));
            if (candidateId == groupV3->getId()) {
                continue;
            }
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
