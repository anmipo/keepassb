/*
 * PwEntry.cpp
 *
 *  Created on: 27 May 2014
 *      Author: Andrei
 */

#include "PwEntry.h"
#include <QDebug>
#include <QDateTime>
#include <QList>
#include "util/Util.h"
#include "db/PwDatabase.h"


/**************************/
PwAttachment::PwAttachment(QObject* parent) :
        QObject(parent),
        name(""),
        data() {
    isCompressed = false;
    isOriginallyCompressed = false;
}

PwAttachment::~PwAttachment() {
    clear();
}

void PwAttachment::clear() {
    Util::safeClear(name);
    Util::safeClear(data);
    isCompressed = false;
    isOriginallyCompressed = false;
}

bool PwAttachment::saveContentToFile(const QString& fileName) {
    qDebug() << "Saving attachment to file: " << fileName;

    if (!inflateData())
        return false;

    QFile outFile(fileName);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing: " << fileName;
        return false;
    }
    qint64 sizeWritten = outFile.write(data);
    if (sizeWritten != data.size()) {
        qDebug("%d bytes written out of %d total", (int)sizeWritten, data.size());
        return false;
    }
    outFile.close();
    return true;
}

void PwAttachment::setName(const QString& name) {
    if (this->name != name) {
        this->name = name;
        emit nameChanged(name);
    }
}

void PwAttachment::setData(const QByteArray& data, const bool isCompressed) {
    this->data = data;
    this->isOriginallyCompressed = isCompressed;
    this->isCompressed = isCompressed;
    emit sizeChanged(data.size()); // FIXME wrong value for compressed data
}

int PwAttachment::getSize() {
    if (isCompressed) {
        bool inflateOk = inflateData();
        if (!inflateOk)
            return -1;
    }
    return data.size();
}

bool PwAttachment::inflateData() {
    if (isCompressed) {
        QByteArray unpackedData;
        Util::ErrorCode err = Util::inflateGZipData(data, unpackedData);
        if (err != Util::SUCCESS) {
            qDebug() << "Attachment inflate error" << err;
            return false;
        }
        data = unpackedData;
        isCompressed = false;
        // the original 'compressed' flag stays in isOriginallyCompressed
        qDebug() << "Data unpacked";
    } else {
        qDebug() << "Data not compressed, no need to inflate";
    }
    return true;
}

/** Returns true if any string contains the query string. */
bool PwAttachment::matchesQuery(const QString& query) const {
    return getName().contains(query, Qt::CaseInsensitive);
}


/**************************/
PwEntry::PwEntry(QObject* parent) : QObject(parent), _uuid(), _iconId(0),
        _creationTime(), _lastModificationTime(),
        _lastAccessTime(), _expiryTime(),
        _expires(false), _deleted(false),
        _parentGroup(NULL), _attachmentsDataModel() {
    _attachmentsDataModel.setParent(this);
}

PwEntry::~PwEntry() {
    clear();
}

void PwEntry::clear() {
    _uuid.clear();
    _iconId = -1;
    _creationTime.setMSecsSinceEpoch(0L);
    _lastModificationTime.setMSecsSinceEpoch(0L);
    _lastAccessTime.setMSecsSinceEpoch(0L);
    _expiryTime.setMSecsSinceEpoch(0L);
    _attachmentsDataModel.clear();
    _deleted = false;
    _expires = false;
    _parentGroup = NULL;
}

QString PwEntry::toString() const {
    return "PwEntry[" + getTitle() + "]";
}

bool PwEntry::lessThan(const PwEntry* e1, const PwEntry* e2) {
    return e1->getTitle().compare(e2->getTitle(), Qt::CaseInsensitive) < 0;
}

bool PwEntry::matchesQuery(const QString& query) const {
    if (getTitle().contains(query, Qt::CaseInsensitive) ||
           getUserName().contains(query, Qt::CaseInsensitive) ||
           getUrl().contains(query, Qt::CaseInsensitive) ||
           getNotes().contains(query, Qt::CaseInsensitive)) {
        return true;
    }
    for (int i = 0; i < _attachmentsDataModel.size(); i++) {
        if (_attachmentsDataModel.value(i)->matchesQuery(query))
            return true;
    }
    return false;
}

/** Updates modification and last access timestamps to current time */
void PwEntry::renewTimestamps() {
    QDateTime now = QDateTime::currentDateTime();
    setLastAccessTime(now);
    setLastModificationTime(now);
}

void PwEntry::setUuid(const PwUuid& uuid) {
    if (uuid != _uuid) {
        _uuid = uuid;
        emit uuidChanged(uuid);
    }
}
void PwEntry::setIconId(int iconId) {
    if (iconId != _iconId) {
        _iconId = iconId;
        emit iconIdChanged(iconId);
    }
}

void PwEntry::setCreationTime(const QDateTime& time) {
    if (time != _creationTime) {
        _creationTime = time;
        emit creationTimeChanged(time);
    }
}

void PwEntry::setLastModificationTime(const QDateTime& time) {
    if (time != _lastModificationTime) {
        _lastModificationTime = time;
        emit lastModificationTimeChanged(time);
    }
}

void PwEntry::setLastAccessTime(const QDateTime& time) {
    if (time != _lastAccessTime) {
        _lastAccessTime = time;
        emit lastAccessTimeChanged(time);
    }
}

void PwEntry::setExpiryTime(const QDateTime& time) {
    if (time != _expiryTime) {
        _expiryTime = time;
        emit expiryTimeChanged(time);
    }
}

void PwEntry::setDeleted(bool deleted) {
    if (deleted != _deleted) {
        _deleted = deleted;
        emit deletedChanged(deleted);
    }
}

void PwEntry::setExpires(bool expires) {
    if (expires != _expires) {
        _expires = expires;
        emit expiresChanged(expires);
    }
}

void PwEntry::setParentGroup(PwGroup* parentGroup) {
    if (parentGroup != _parentGroup) {
        _parentGroup = parentGroup;
        emit parentGroupChanged(parentGroup);
    }
}

void PwEntry::addAttachment(PwAttachment* attachment) {
    int prevSize = _attachmentsDataModel.size();
    _attachmentsDataModel.append(attachment); // implicitly takes ownership
    int newSize = _attachmentsDataModel.size();
    if (newSize != prevSize) {
        emit attachmentCountChanged(newSize);
    }
}

/** Removes the entry from the parent group. Does NOT make a copy in Backup/Recycle bin. */
void PwEntry::deleteWithoutBackup() {
    if (_parentGroup) {
        _parentGroup->removeEntry(this);
    }
}

/**
 * Moves the entry to Backup/Recycle group.
 * Returns true if successful.
 */
bool PwEntry::moveToBackup() {
    PwGroup* parentGroup = getParentGroup();
    if (!parentGroup) {
        qDebug() << "moveToBackup fail - no parent group set";
        return false;
    }

    PwDatabase* db = parentGroup->getDatabase();
    if (!db) {
        qDebug() << "moveToBackup fail - parent group without DB";
        return false;
    }

    PwGroup* backupGroup = db->getBackupGroup(true);
    if (!backupGroup) {
        qDebug() << "moveToBackup fail - no backup group created";
        return false;
    }

    parentGroup->removeEntry(this);
    backupGroup->addEntry(this);

    setParent(backupGroup); // parent in Qt terms, responsible for memory release
    setDeleted(true);

    qDebug() << "moveToBackup OK";
    return true;
}
