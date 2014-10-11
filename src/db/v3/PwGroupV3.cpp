/*
 * PwGroupV3.cpp
 *
 *  Created on: 19 Aug 2014
 *      Author: Andrei
 */

#include <PwGroupV3.h>
#include "util/Util.h"
#include "db/v3/PwStreamUtilsV3.h"
#include "db/v3/PwEntryV3.h"
#include "db/v3/PwDatabaseV3.h"

const QString PwGroupV3::BACKUP_GROUP_NAME = QString("Backup");

PwGroupV3::PwGroupV3(QObject* parent) : PwGroup(parent) {
    _id = -1;
    _level = -1;
    _flags = 0;
}

PwGroupV3::~PwGroupV3() {
    clear();
}

void PwGroupV3::clear() {
    _id = -1;
    _level = -1;
    _flags = 0;
    PwGroup::clear();
}

void PwGroupV3::addSubGroup(PwGroup* subGroup) {
    PwGroup::addSubGroup(subGroup);
    dynamic_cast<PwGroupV3*>(subGroup)->setLevel(this->getLevel() + 1);
}
void PwGroupV3::removeSubGroup(PwGroup* subGroup) {
    PwGroup::removeSubGroup(subGroup);
    // there is no suitable level value, so just reset to zero (root level) to simplify debug
    dynamic_cast<PwGroupV3*>(subGroup)->setLevel(0);
}
void PwGroupV3::addEntry(PwEntry* entry) {
    PwGroup::addEntry(entry);
    dynamic_cast<PwEntryV3*>(entry)->setGroupId(this->getId());
}
void PwGroupV3::removeEntry(PwEntry* entry) {
    PwGroup::removeEntry(entry);
    // there is no suitable groupId, so just reset to zero to simplify debug
    dynamic_cast<PwEntryV3*>(entry)->setGroupId(0);
}

/**
 * Creates an entry in the group and returns a reference to it.
 */
PwEntry* PwGroupV3::createEntry() {
    PwEntryV3* newEntry = new PwEntryV3(this);
    newEntry->setUuid(PwUuid::create());

    // inherit the icon and recycled status
    newEntry->setIconId(this->getIconId());
    newEntry->setDeleted(this->isDeleted());

    // set times
    newEntry->setCreationTime(QDateTime::currentDateTime());
    newEntry->setLastAccessTime(QDateTime::currentDateTime());
    newEntry->setLastModificationTime(QDateTime::currentDateTime());
    // newEntry->setExpires(false); <- in V3 is managed by setExpiryTime()
    newEntry->setExpiryTime(PwDatabaseV3::EXPIRY_DATE_NEVER);

    // set parent group
    newEntry->setGroupId(this->getId());
    newEntry->setParentGroup(this);
    this->addEntry(newEntry);
    return newEntry;
}

/**
 * Creates a subgroup in the group and returns a reference to it.
 */
PwGroup* PwGroupV3::createGroup() {
    PwGroupV3* newGroup = new PwGroupV3(this);
    newGroup->setUuid(PwUuid::create());

    newGroup->setFlags(0);

    // create an ID that does not exist already
    qint32 newId = reinterpret_cast<PwDatabaseV3*>(getDatabase())->createNewGroupId();
    newGroup->setId(newId);

    // inherit the icon and recycled status
    newGroup->setIconId(this->getIconId());
    newGroup->setDeleted(this->isDeleted());

    // set times
    newGroup->setCreationTime(QDateTime::currentDateTime());
    newGroup->setLastAccessTime(QDateTime::currentDateTime());
    newGroup->setLastModificationTime(QDateTime::currentDateTime());
    // newGroup->setExpires(false); <- in V3 is managed by setExpiryTime()
    newGroup->setExpiryTime(PwDatabaseV3::EXPIRY_DATE_NEVER);

    // set parent group
    newGroup->setParentGroup(this);
    newGroup->setLevel(this->getLevel() + 1);
    this->addSubGroup(newGroup);
    return newGroup;
}

/**
 * Recursively iterates through all the children groups and entries of this group
 * and adds them to the given lists. The group itself is excluded.
 */
void PwGroupV3::getAllChildren(QList<PwGroupV3*> &childGroups, QList<PwEntryV3*> &childEntries) const {
    QList<PwGroup*> groups = this->getSubGroups();
    for (int i = 0; i < groups.size(); i++) {
        PwGroupV3* gr = dynamic_cast<PwGroupV3*>(groups.at(i));
        childGroups.append(gr);
        gr->getAllChildren(childGroups, childEntries);
    }
    QList<PwEntry*> entries = this->getEntries();
    for (int i = 0; i < entries.size(); i++) {
        childEntries.append(reinterpret_cast<PwEntryV3*>(entries.at(i)));
    }
}

/** Loads group fields from the stream. Returns true on success, false in case of error. */
bool PwGroupV3::readFromStream(QDataStream& stream) {
    quint16 fieldType;
    qint32 fieldSize;
    while (!stream.atEnd()) {
        stream >> fieldType >> fieldSize;
        switch(fieldType) {
        case FIELD_RESERVED:
            stream.skipRawData(fieldSize);
            break;
        case FIELD_GROUP_ID:
            this->setId(PwStreamUtilsV3::readInt32(stream));
            break;
        case FIELD_NAME:
            this->setName(PwStreamUtilsV3::readString(stream, fieldSize));
            break;
        case FIELD_CREATION_TIME:
            this->setCreationTime(PwStreamUtilsV3::readTimestamp(stream));
            break;
        case FIELD_LAST_MODIFIED_TIME:
            this->setLastModificationTime(PwStreamUtilsV3::readTimestamp(stream));
            break;
        case FIELD_LAST_ACCESS_TIME:
            this->setLastAccessTime(PwStreamUtilsV3::readTimestamp(stream));
            break;
        case FIELD_EXPIRATION_TIME:
            this->setExpiryTime(PwStreamUtilsV3::readTimestamp(stream));
            break;
        case FIELD_ICON_ID:
            this->setIconId(PwStreamUtilsV3::readInt32(stream));
            break;
        case FIELD_GROUP_LEVEL:
            this->setLevel(PwStreamUtilsV3::readUInt16(stream));
            break;
        case FIELD_GROUP_FLAGS:
            this->setFlags(PwStreamUtilsV3::readInt32(stream));
            break;
        case FIELD_END:
            // group fields finished
            stream.skipRawData(fieldSize);
            // a "Backup" group in the root is equivalent of V4's "Recycle Bin"
            if ((this->getLevel() == 0) && (BACKUP_GROUP_NAME == this->getName())) {
                this->setDeleted(true);
            }
            qDebug() << "Load group: '" << getName() << "' id:" << getId();
            return true;
        }
    }
    // something went wrong, there was no FIELD_END marker
    return false;
}

/** Writes group fields to the stream. Returns true on success, false in case of error. */
bool PwGroupV3::writeToStream(QDataStream& stream) {
    stream << FIELD_GROUP_ID;
    PwStreamUtilsV3::writeInt32(stream, getId());

    stream << FIELD_NAME;
    PwStreamUtilsV3::writeString(stream, getName());

    stream << FIELD_CREATION_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getCreationTime());

    stream << FIELD_LAST_MODIFIED_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getLastModificationTime());

    stream << FIELD_LAST_ACCESS_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getLastAccessTime());

    stream << FIELD_EXPIRATION_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getExpiryTime());

    stream << FIELD_ICON_ID;
    PwStreamUtilsV3::writeInt32(stream, getIconId());

    stream << FIELD_GROUP_LEVEL;
    PwStreamUtilsV3::writeUInt16(stream, getLevel());

    stream << FIELD_GROUP_FLAGS;
    PwStreamUtilsV3::writeInt32(stream, getFlags());

    stream << FIELD_END << (qint32)0;

    qDebug() << "Save group: '" << getName() << "' id:" << getId();
    return (stream.status() == QDataStream::Ok);
}

/**
 * Checks if a group name is reserved for internal use and cannot be assigned by the user.
 */
bool PwGroupV3::isNameReserved(const QString& name) {
    return (BACKUP_GROUP_NAME == name);
}
