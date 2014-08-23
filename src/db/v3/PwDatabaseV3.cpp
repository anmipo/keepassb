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

PwHeaderV3::PwHeaderV3(QObject* parent) : QObject(parent),
        masterSeed(), initialVector(), contentHash(), transformSeed() {
    transformRounds = 0;
    groupCount = 0;
    entryCount = 0;
}

PwHeaderV3::~PwHeaderV3() {
    clear();
}

void PwHeaderV3::clear() {
    masterSeed.clear();
    initialVector.clear();
    contentHash.clear();
    transformSeed.clear();
    transformRounds = 0;
    groupCount = 0;
    entryCount = 0;
}

PwHeaderV3::ErrorCode PwHeaderV3::read(QDataStream& stream) {
    clear();

    // check file signatures (although probably checked before)
    quint32 sign1, sign2, flags, fileVersion;
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

    masterSeed.fill(0, 16);
    stream.readRawData(masterSeed.data(), 16);
    initialVector.fill(0, 16);
    stream.readRawData(initialVector.data(), 16);

    stream >> groupCount >> entryCount;

    contentHash.fill(0, 32);
    stream.readRawData(contentHash.data(), 32);
    transformSeed.fill(0, 32);
    stream.readRawData(transformSeed.data(), 32);

    stream >> transformRounds;
    return SUCCESS;
}

QString PwHeaderV3::getErrorMessage(ErrorCode errCode) {
    switch (errCode) {
    case SUCCESS:
        return "";
    case SIGNATURE_1_MISMATCH:
        // fallthrough
    case SIGNATURE_2_MISMATCH:
        return tr("Wrong file signature");
    case UNSUPPORTED_FILE_VERSION:
        return tr("Unsupported DB file version");
    case NOT_AES:
        return tr("Twofish cypher is not supported");
    default:
        return tr("Header error");
    }
}


PwDatabaseV3::PwDatabaseV3(QObject* parent) : PwDatabase(parent),
        header(), combinedKey(), aesKey(), metaStreamEntries() {
    header.setParent(this);
}

PwDatabaseV3::~PwDatabaseV3() {
    // nothing to do here
}

void PwDatabaseV3::clear() {
    qDeleteAll(metaStreamEntries);
    metaStreamEntries.clear();
    header.clear();
    combinedKey.clear();
    aesKey.clear();
    PwDatabase::clear();
}

bool PwDatabaseV3::isSignatureMatch(const QByteArray& rawDbData) {
    QDataStream stream(rawDbData);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint32 sign1, sign2;
    stream >> sign1 >> sign2;

    return (sign1 ==  PwHeaderV3::SIGNATURE_1) && (sign2 == PwHeaderV3::SIGNATURE_2);
}

void PwDatabaseV3::unlock(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) {
    clear();

    if (!buildCompositeKey(password.toLatin1(), keyFileData, combinedKey)) {
        emit dbUnlockError(tr("Crypto library error"), COMPOSITE_KEY_ERROR);
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
        emit dbUnlockError(PwHeaderV3::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }
    emit unlockProgressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Calculate the AES key */
    ErrorCode err = transformKey(combinedKey, aesKey, UNLOCK_PROGRESS_HEADER_READ, UNLOCK_PROGRESS_KEY_TRANSFORMED);
    if (err != SUCCESS) {
        qDebug() << "Cannot decrypt database - transformKey" << err;
        emit dbUnlockError(tr("Cannot decrypt database"), err);
        return false;
    }

    emit unlockProgressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Decrypt data */
    int dataSize = dbBytes.size() - header.HEADER_SIZE;
    // DB header not needed for decryption
    QByteArray dbBytesWithoutHeader = dbBytes.right(dataSize);
    QByteArray decryptedData (dataSize, 0);
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
            emit dbUnlockError(tr("Cannot decrypt database"), err);
        }
        return false;
    }
    emit unlockProgressChanged(UNLOCK_PROGRESS_DECRYPTED);

    QDataStream decryptedDataStream(decryptedData);
    decryptedDataStream.setByteOrder(QDataStream::LittleEndian);
    err = readContent(decryptedDataStream);
    if (err != SUCCESS) {
        emit dbUnlockError(tr("Cannot parse database"), err);
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
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_1;

    // clean up
    if (cm->endKeyTransform() != SB_SUCCESS)
        return KEY_TRANSFORM_END_ERROR;

    QByteArray prefinalKey;
    ec = cm->sha256(transformedKey, prefinalKey);
    if (ec != SB_SUCCESS)
        return KEY_TRANSFORM_ERROR_2;

    prefinalKey.prepend(header.getMasterSeed());
    ec = cm->sha256(prefinalKey, aesKey);
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
        err = readGroup(stream, *group);
        if (err != SUCCESS)
            return err;
        qDebug() << "Group " << group->getName() << " id:" << group->getId() << " level:" << group->getLevel();
        groups.append(group);
    }
    return SUCCESS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readGroup(QDataStream& stream, PwGroupV3& group) {
    quint16 fieldType;
    qint32 fieldSize;
    while (!stream.atEnd()) {
        stream >> fieldType >> fieldSize;
        switch(fieldType) {
        case 0x0000: // ignored
            stream.skipRawData(fieldSize);
            break;
        case 0x0001: // group ID
            qint32 groupId;
            stream >> groupId;
            group.setId(groupId);
            break;
        case 0x0002: { // name
            QByteArray nameBuf(fieldSize, 0);
            stream.readRawData(nameBuf.data(), fieldSize);
            QString name = QString::fromUtf8(nameBuf.constData());
            //TODO check reading of unicode strings
            group.setName(name);
            break;
        }
        case 0x0003: // creation time
            group.setCreationTime(readTimestamp(stream));
            break;
        case 0x0004: // last modification time
            group.setLastModificationTime(readTimestamp(stream));
            break;
        case 0x0005: // last access time
            group.setLastAccessTime(readTimestamp(stream));
            break;
        case 0x0006: // expiration time
            group.setExpiryTime(readTimestamp(stream));
            break;
        case 0x0007: { // icon ID
            qint32 iconId;
            stream >> iconId;
            group.setIconId(iconId);
            break;
        }
        case 0x0008: { // group level
            quint16 level;
            stream >> level;
            group.setLevel(level);
            break;
        }
        case 0x0009: { // group flags
            qint32 flags;
            stream >> flags;
            group.setFlags(flags);
            break;
        }
        case 0xFFFF:
            // group fields finished
            stream.skipRawData(fieldSize);
            return SUCCESS;
        }
    }
    // if we reach here, something went wrong
    return NOT_ENOUGH_GROUPS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readAllEntries(QDataStream& stream, QList<PwEntryV3*> &entries) {
    PwDatabaseV3::ErrorCode err;
    for (quint32 iEntry = 0; iEntry < header.getEntryCount(); iEntry++) {
        PwEntryV3* entry = new PwEntryV3();
        err = readEntry(stream, *entry);
        if (err != SUCCESS)
            return err;
        entries.append(entry);
    }
    return SUCCESS;
}

PwDatabaseV3::ErrorCode PwDatabaseV3::readEntry(QDataStream& stream, PwEntryV3& entry) {
    quint16 fieldType;
    qint32 fieldSize;
    while (!stream.atEnd()) {
        stream >> fieldType >> fieldSize;
        switch(fieldType) {
        case 0x0000: // ignored
            stream.skipRawData(fieldSize);
            break;
        case 0x0001: { // UUID
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            entry.setUuid(PwUuid(buf));
            break;
        }
        case 0x0002: { // group ID
            qint32 groupId;
            stream >> groupId;
            entry.setGroupId(groupId);
            break;
        }
        case 0x0003: { // icon ID
            qint32 iconId;
            stream >> iconId;
            entry.setIconId(iconId);
            break;
        }
        case 0x0004: { // title
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString title = QString::fromUtf8(buf.constData());
            //TODO check reading of unicode strings
            entry.setTitle(title);
            break;
        }
        case 0x0005: { // url
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString url = QString::fromUtf8(buf.constData()); // check with unicode
            entry.setUrl(url);
            break;
        }
        case 0x0006: { // username
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString username = QString::fromUtf8(buf.constData());
            entry.setUserName(username);
            break;
        }
        case 0x0007: { // password
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            entry.setPassword(QString::fromUtf8(buf.constData())); // TODO check with unicode
            break;
        }
        case 0x0008: { // note
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString note = QString::fromUtf8(buf.constData());
            entry.setNotes(note);
            break;
        }
        case 0x0009: // creation time
            entry.setCreationTime(readTimestamp(stream));
            break;
        case 0x000A: // last modification time
            entry.setLastModificationTime(readTimestamp(stream));
            break;
        case 0x000B: // last access time
            entry.setLastAccessTime(readTimestamp(stream));
            break;
        case 0x000C: // expiration time
            entry.setExpiryTime(readTimestamp(stream));
            break;
        case 0x000D: { // binary description
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString binaryDesc = QString::fromUtf8(buf.constData());
            entry.setBinaryDesc(binaryDesc);
            break;
        }
        case 0x000E: { // binary data
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            entry.setBinaryData(buf); // TODO must deep-copy
            break;
        }
        case 0xFFFF:
            // group fields finished
            stream.skipRawData(fieldSize);
            if (!entry.getBinaryData().isEmpty()) {
                // make the binary data available via the common 'attachment' interface
                PwAttachment* attachment = new PwAttachment(&entry);
                attachment->setName(entry.getBinaryDesc());
                attachment->setData(entry.getBinaryData(), false);
                entry.addAttachment(attachment);
            }
            return SUCCESS;
        }
    }
    // if we reach here, something went wrong
    return NOT_ENOUGH_GROUPS;
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
