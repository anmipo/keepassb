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


const int PwEntry::DEFAULT_ICON_ID = 0; // "key"

/*****************************/

PwAttachment::PwAttachment(QObject* parent) :
        QObject(parent),
        _isInitialized(false),
        _id(-1),
        _name(""),
        _isCompressed(false),
        _uncompressedSize(-1),
        _data() {
     // nothing else to do
}

PwAttachment::~PwAttachment() {
    clear();
}

void PwAttachment::clear() {
    _id = -1;
    Util::safeClear(_name);
    Util::safeClear(_data);
    _uncompressedSize = -1;
    _isCompressed = false;
    _isInitialized = false;
}

bool PwAttachment::saveContentToFile(const QString& fileName) {
    qDebug() << "Saving attachment to file: " << fileName;

    QFile outFile(fileName);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing: " << fileName;
        return false;
    }

    int dataSize, sizeWritten;
    if (_isCompressed) {
        QByteArray unpackedData;
        Util::ErrorCode err = Util::inflateGZipData(_data, unpackedData);
        if (err != Util::SUCCESS) {
            qDebug() << "Attachment inflate error" << err;
            return false;
        }
        qDebug() << "Data unpacked";

        dataSize = unpackedData.size();
        sizeWritten = outFile.write(unpackedData);
        // safely clean up temporary data from memory
        Util::safeClear(unpackedData);
    } else {
        dataSize = _data.size();
        sizeWritten = outFile.write(_data);
        // do not clear data -- it is a class field
    }

    if (sizeWritten != dataSize) {
        qDebug("%d bytes written out of %d total", sizeWritten, dataSize);
        return false;
    }
    outFile.close();
    return true;
}

void PwAttachment::setName(const QString& name) {
    if (_name != name) {
        _isInitialized = true;
        _name = name;
        emit nameChanged(name);
    }
}

void PwAttachment::setData(const QByteArray& data, const bool isCompressed) {
    _isInitialized = true;
    // Store a clone copy to avoid surprises if external data is changed/cleared
    this->_data = QByteArray::fromRawData(data.constData(), data.size());
    this->_isCompressed = isCompressed;
    emit sizeChanged(getSize());
}

void PwAttachment::setId(const int newId) {
    if (_id != newId) {
        _id = newId;
        emit idChanged(newId);
    }
}

/** Returns size of _uncompressed_ data. */
int PwAttachment::getSize() {
    if (_uncompressedSize < 0) {
        if (_isCompressed) {
            // estimate the size once and cache the value
            _uncompressedSize = Util::getInflatedGZipSize(_data);
        } else {
            _uncompressedSize = _data.size();
        }
    }
    return _uncompressedSize;
}

/** Returns true if any string contains the query string. */
bool PwAttachment::matchesQuery(const QString& query) const {
    return getName().contains(query, Qt::CaseInsensitive);
}

PwAttachment* PwAttachment::createFromFile(const QString& filePath) {
    // Load the file to memory
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open attachment file: '" << filePath << "' Error: " << file.error() << ". Message: " << file.errorString();
        return NULL;
    }
    QByteArray fileData = file.readAll();
    if (file.error() != QFile::NoError) {
        qDebug() << "Cannot read attachment file: '" << filePath << "' Error: " << file.error() << ". Message: " << file.errorString();
        file.close();
        return NULL;
    }
    file.close();

    // allow empty files, since we won't be able to show a meaningful error
    QFileInfo fileInfo(file);
    QString fileName = fileInfo.fileName();

    //TODO try compressing data here

    PwAttachment* result = new PwAttachment();
    // the ownership will be later taken by the data model
    result->setData(fileData, false);
    result->setName(fileName);

    return result;
}

/*****************************/

PwAttachmentDataModel::PwAttachmentDataModel(QObject* parent) : QListDataModel<PwAttachment*> () {
    setParent(parent);
    _size = 0;

    bool res = QObject::connect(this, SIGNAL(itemAdded(QVariantList)), this, SLOT(updateSize())); Q_ASSERT(res);
    res = QObject::connect(this, SIGNAL(itemUpdated(QVariantList)), this, SLOT(updateSize())); Q_ASSERT(res);
    res = QObject::connect(this, SIGNAL(itemRemoved(QVariantList)), this, SLOT(updateSize())); Q_ASSERT(res);
    res = QObject::connect(this, SIGNAL(itemsChanged(bb::cascades::DataModelChangeType::Type, QSharedPointer<bb::cascades::DataModel::IndexMapper>)),
            this, SLOT(updateSize())); Q_ASSERT(res);
    Q_UNUSED(res);
}

void PwAttachmentDataModel::updateSize() {
    if (_size != size()) {
        _size = size();
        emit sizeChanged(_size);
    }
}

void PwAttachmentDataModel::clear() {
    int size = this->size();
    for (int i = 0; i < size; i++) {
        PwAttachment* att = value(i);
        att->clear();
        att->deleteLater();
    }
    QListDataModel<PwAttachment*>::clear();
}

void PwAttachmentDataModel::removeAt(int index) {
    PwAttachment* att = this->value(index);
    QListDataModel<PwAttachment*>::removeAt(index);
    att->clear();
    att->deleteLater();
}

/*****************************/

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

    QDateTime now = QDateTime::currentDateTime();
    _creationTime = now;
    _lastModificationTime = now;
    _lastAccessTime = now;
    _expiryTime = now;

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

bool PwEntry::isExpired() const {
    return isExpires() && (QDateTime::currentDateTime() > getExpiryTime());
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
        bool wasExpired = isExpired();
        _expiryTime = time;
        emit expiryTimeChanged(time);
        if (wasExpired != isExpired()) {
            emit expiredChanged(isExpired());
        }
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
        bool wasExpired = isExpired();
        _expires = expires;
        emit expiresChanged(expires);
        if (wasExpired != isExpired()) {
            emit expiredChanged(isExpired());
        }
    }
}

void PwEntry::setParentGroup(PwGroup* parentGroup) {
    if (parentGroup != _parentGroup) {
        _parentGroup = parentGroup;
        emit parentGroupChanged(parentGroup);
    }
}

bool PwEntry::addAttachment(PwAttachment* attachment) {
    _attachmentsDataModel.append(attachment); // implicitly takes ownership
    return true;
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
