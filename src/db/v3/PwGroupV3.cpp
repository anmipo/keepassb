/*
 * PwGroupV3.cpp
 *
 *  Created on: 19 Aug 2014
 *      Author: Andrei
 */

#include <PwGroupV3.h>
#include "util/Util.h"
#include "PwDatabaseV3.h"

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
            qint32 groupId;
            stream >> groupId;
            this->setId(groupId);
            break;
        case FIELD_NAME: {
            QByteArray nameBuf(fieldSize, 0);
            stream.readRawData(nameBuf.data(), fieldSize);
            QString name = QString::fromUtf8(nameBuf.constData());
            this->setName(name);
            Util::safeClear(nameBuf);
            break;
        }
        case FIELD_CREATION_TIME:
            this->setCreationTime(PwDatabaseV3::readTimestamp(stream));
            break;
        case FIELD_LAST_MODIFIED_TIME:
            this->setLastModificationTime(PwDatabaseV3::readTimestamp(stream));
            break;
        case FIELD_LAST_ACCESS_TIME:
            this->setLastAccessTime(PwDatabaseV3::readTimestamp(stream));
            break;
        case FIELD_EXPIRATION_TIME:
            this->setExpiryTime(PwDatabaseV3::readTimestamp(stream));
            break;
        case FIELD_ICON_ID: {
            qint32 iconId;
            stream >> iconId;
            this->setIconId(iconId);
            break;
        }
        case FIELD_GROUP_LEVEL: {
            quint16 level;
            stream >> level;
            this->setLevel(level);
            break;
        }
        case FIELD_GROUP_FLAGS: {
            qint32 flags;
            stream >> flags;
            this->setFlags(flags);
            break;
        }
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
