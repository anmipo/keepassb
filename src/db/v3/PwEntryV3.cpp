/*
 * PwEntryV3.cpp
 *
 *  Created on: 21 Aug 2014
 *      Author: Andrei
 */

#include <PwEntryV3.h>
#include "util/Util.h"
#include "db/v3/PwStreamUtilsV3.h"
#include "db/v3/PwDatabaseV3.h"

// field values of meta-stream entries
const QString METASTREAM_ID_TITLE = QString("Meta-Info");
const QString METASTREAM_ID_USER = QString("SYSTEM");
const QString METASTREAM_ID_URL = QString("$");
const QString METASTREAM_ID_BINDESC = QString("bin-stream");

PwEntryV3::PwEntryV3(QObject* parent) : PwEntry(parent), _binaryDesc(), _binaryData(),
        _title(), _userName(), _password(), _url(), _notes() {
    _groupId = 0;
}

PwEntryV3::~PwEntryV3() {
    // nothing to do here
}

void PwEntryV3::clear() {
    _groupId = 0;
    Util::safeClear(_binaryDesc);
    Util::safeClear(_binaryData);
    Util::safeClear(_title);
    Util::safeClear(_userName);
    Util::safeClear(_password);
    Util::safeClear(_url);
    Util::safeClear(_notes);
    PwEntry::clear();
}

bool PwEntryV3::matchesQuery(const QString& query) const {
    return PwEntry::matchesQuery(query) ||
            getBinaryDesc().contains(query, Qt::CaseInsensitive);
}

void PwEntryV3::addAttachment(PwAttachment* attachment) {
    // V3 entries can only have one attachment
    Q_ASSERT(getAttachmentCount() == 0);
    PwEntry::addAttachment(attachment);
}

bool PwEntryV3::isMetaStream() const {
    if (_notes.isEmpty() || _binaryData.isNull() || _binaryDesc.isEmpty())
        return false;

    bool cond1 = (_binaryDesc == METASTREAM_ID_BINDESC);
    bool cond2 = (_userName == METASTREAM_ID_USER);
    bool cond3 = (_url == METASTREAM_ID_URL);
    bool cond4 = (_title == METASTREAM_ID_TITLE);

    return (getIconId() == 0 && cond1 && cond2 && cond3 && cond4);
}

void PwEntryV3::setGroupId(qint32 groupId) {
    if (groupId != _groupId) {
        _groupId = groupId;
        emit groupIdChanged(groupId);
    }
}

void PwEntryV3::setBinaryDesc(QString desc) {
    if (desc != _binaryDesc) {
        _binaryDesc = desc;
        emit binaryDescChanged(desc);
    }
}

void PwEntryV3::setBinaryData(const QByteArray& data) {
    if (data != _binaryData) {
        _binaryData = data;
        emit binaryDataChanged(data);
    }
}

void PwEntryV3::setTitle(const QString& title) {
    if (title != _title) {
        _title = title;
        emit titleChanged(title);
    }
}

void PwEntryV3::setUserName(const QString& userName) {
    if (userName != _userName) {
        _userName = userName;
        emit userNameChanged(userName);
    }
}

void PwEntryV3::setPassword(const QString& password) {
    if (password != _password) {
        _password = password;
        emit passwordChanged(password);
    }
}

void PwEntryV3::setUrl(const QString& url) {
    if (url != _url) {
        _url = url;
        emit urlChanged(url);
    }
}

void PwEntryV3::setNotes(const QString& notes) {
    if (notes != _notes) {
        _notes = notes;
        emit notesChanged(notes);
    }
}

void PwEntryV3::setExpiryTime(const QDateTime& time) {
    PwEntry::setExpiryTime(time);
    setExpires(time != PwDatabaseV3::EXPIRY_DATE_NEVER);
}

/** Loads entry fields from the stream. Returns true on success, false in case of error. */
bool PwEntryV3::readFromStream(QDataStream& stream) {
    clear();
    quint16 fieldType;
    qint32 fieldSize;
    while (!stream.atEnd()) {
        stream >> fieldType >> fieldSize;
        switch(fieldType) {
        case FIELD_RESERVED: // ignored
            stream.skipRawData(fieldSize);
            break;
        case FIELD_UUID:
            this->setUuid(PwUuid(PwStreamUtilsV3::readData(stream, fieldSize)));
            break;
        case FIELD_GROUP_ID:
            this->setGroupId(PwStreamUtilsV3::readInt32(stream));
            break;
        case FIELD_ICON_ID:
            this->setIconId(PwStreamUtilsV3::readInt32(stream));
            break;
        case FIELD_TITLE:
            this->setTitle(PwStreamUtilsV3::readString(stream, fieldSize));
            break;
        case FIELD_URL:
            this->setUrl(PwStreamUtilsV3::readString(stream, fieldSize));
            break;
        case FIELD_USERNAME:
            this->setUserName(PwStreamUtilsV3::readString(stream, fieldSize));
            break;
        case FIELD_PASSWORD:
            this->setPassword(PwStreamUtilsV3::readString(stream, fieldSize));
            break;
        case FIELD_NOTE:
            this->setNotes(PwStreamUtilsV3::readString(stream, fieldSize));
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
        case FIELD_BINARY_DESC:
            this->setBinaryDesc(PwStreamUtilsV3::readString(stream, fieldSize));
            break;
        case FIELD_BINARY_DATA: {
            this->setBinaryData(PwStreamUtilsV3::readData(stream, fieldSize));
            break;
        }
        case FIELD_END:
            // group fields finished
            stream.skipRawData(fieldSize);
            if (!this->getBinaryData().isEmpty()) {
                // make the binary data available via the common 'attachment' interface
                PwAttachment* attachment = new PwAttachment(this);
                attachment->setName(this->getBinaryDesc());
                attachment->setData(this->getBinaryData(), false);
                this->addAttachment(attachment);
            }
            qDebug() << "Load entry: '" << getTitle() << "' groupId:" << getGroupId();
            return true;
        }
    }
    return false;
}

/** Writes entry fields to the stream. Returns true on success, false in case of error. */
bool PwEntryV3::writeToStream(QDataStream& stream) {
    stream << FIELD_UUID;
    PwStreamUtilsV3::writeData(stream, getUuid().toByteArray());

    stream << FIELD_GROUP_ID;
    PwStreamUtilsV3::writeInt32(stream, getGroupId());

    stream << FIELD_ICON_ID;
    PwStreamUtilsV3::writeInt32(stream, getIconId());

    stream << FIELD_TITLE;
    PwStreamUtilsV3::writeString(stream, getTitle());

    stream << FIELD_URL;
    PwStreamUtilsV3::writeString(stream, getUrl());

    stream << FIELD_USERNAME;
    PwStreamUtilsV3::writeString(stream, getUserName());

    stream << FIELD_PASSWORD;
    PwStreamUtilsV3::writeString(stream, getPassword());

    stream << FIELD_NOTE;
    PwStreamUtilsV3::writeString(stream, getNotes());

    stream << FIELD_CREATION_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getCreationTime());

    stream << FIELD_LAST_MODIFIED_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getLastModificationTime());

    stream << FIELD_LAST_ACCESS_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getLastAccessTime());

    stream << FIELD_EXPIRATION_TIME;
    PwStreamUtilsV3::writeTimestamp(stream, getExpiryTime());

    stream << FIELD_BINARY_DESC;
    PwStreamUtilsV3::writeString(stream, getBinaryDesc());

    stream << FIELD_BINARY_DATA;
    PwStreamUtilsV3::writeData(stream, getBinaryData());

    stream << FIELD_END << (qint32)0;

    qDebug() << "Save entry: '" << getTitle() << "' groupId:" << getGroupId();
    return (stream.status() == QDataStream::Ok);
}

/** Returns a new entry instance with the same field values. */
PwEntry* PwEntryV3::clone() {
    QByteArray buffer;
    QDataStream bufferStream(&buffer, QIODevice::ReadWrite);
    if (!writeToStream(bufferStream)) {
        qDebug() << "PwEntryV3::clone() failed on write";
        return NULL;
    }

    bufferStream.device()->reset();
    PwEntryV3* copy = new PwEntryV3(this->parent());
    if (!copy->readFromStream(bufferStream)) {
        delete copy;
        qDebug() << "PwEntryV3::clone() failed on read";
        return NULL;
    }
    // insert copy into the same group (readFromStream does not do that)
    copy->setParentGroup(getParentGroup());

    return copy;
}

/**
 * Makes a backup copy of the current values/state of the entry.
 * Returns true if successful.
 */
bool PwEntryV3::backupState() {
    // KeePass 1 backups entry state by copying the whole entry to the Backup group.
    PwEntry* entryCopy = this->clone();
    return entryCopy->moveToBackup();
}
