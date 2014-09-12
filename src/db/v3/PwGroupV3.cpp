/*
 * PwGroupV3.cpp
 *
 *  Created on: 19 Aug 2014
 *      Author: Andrei
 */

#include <PwGroupV3.h>
#include "util/Util.h"
#include "db/v3/PwStreamUtilsV3.h"

const QString BACKUP_GROUP_NAME = QString("Backup");

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

    return (stream.status() == QDataStream::Ok);
}
