/*
 * PwEntryV3.cpp
 *
 *  Created on: 21 Aug 2014
 *      Author: Andrei
 */

#include <PwEntryV3.h>
#include "util/Util.h"
#include "PwDatabaseV3.h"

// field values of meta-stream entries
const QString METASTREAM_ID_TITLE = QString("Meta-Info");
const QString METASTREAM_ID_USER = QString("SYSTEM");
const QString METASTREAM_ID_URL = QString("$");
const QString METASTREAM_ID_BINDESC = QString("bin-stream");

const QDateTime EXPIRY_DATE_NEVER = QDateTime(QDate(2999, 12, 28), QTime(23, 59, 59, 0), Qt::UTC);

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
        case FIELD_UUID: {
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            this->setUuid(PwUuid(buf));
            Util::safeClear(buf);
            break;
        }
        case FIELD_GROUP_ID: {
            qint32 groupId;
            stream >> groupId;
            this->setGroupId(groupId);
            break;
        }
        case FIELD_ICON_ID: {
            qint32 iconId;
            stream >> iconId;
            this->setIconId(iconId);
            break;
        }
        case FIELD_TITLE: {
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString title = QString::fromUtf8(buf.constData());
            this->setTitle(title);
            Util::safeClear(buf);
            break;
        }
        case FIELD_URL: {
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString url = QString::fromUtf8(buf.constData()); // check with unicode
            this->setUrl(url);
            Util::safeClear(buf);
            break;
        }
        case FIELD_USERNAME: {
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString username = QString::fromUtf8(buf.constData());
            this->setUserName(username);
            Util::safeClear(buf);
            break;
        }
        case FIELD_PASSWORD: {
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            this->setPassword(QString::fromUtf8(buf.constData()));
            Util::safeClear(buf);
            break;
        }
        case FIELD_NOTE: {
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString note = QString::fromUtf8(buf.constData());
            this->setNotes(note);
            Util::safeClear(buf);
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
        case FIELD_BINARY_DESC: { // binary description
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            QString binaryDesc = QString::fromUtf8(buf.constData());
            this->setBinaryDesc(binaryDesc);
            Util::safeClear(buf);
            break;
        }
        case FIELD_BINARY_DATA: {
            QByteArray buf(fieldSize, 0);
            stream.readRawData(buf.data(), fieldSize);
            this->setBinaryData(buf);
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
            return true;
        }
    }
    return false;
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
    setExpires(time != EXPIRY_DATE_NEVER);
}
