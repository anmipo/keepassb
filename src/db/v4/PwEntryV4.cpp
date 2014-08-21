/*
 * PwEntryV4.cpp
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#include "db/v4/PwEntryV4.h"
#include "util/Util.h"

// Standard entry fields
const static QString TITLE = "Title";
const static QString USERNAME = "UserName";
const static QString PASSWORD = "Password";
const static QString URL = "URL";
const static QString NOTES = "Notes";


PwExtraField::PwExtraField(QObject* parent) : QObject(parent) {
    _name = "";
    _value = "";
}

PwExtraField::PwExtraField(QObject* parent, const QString& name, const QString& value) : QObject(parent) {
    _name = name;
    _value = value;
}

bool PwExtraField::matchesQuery(const QString& query) const {
    return getName().contains(query, Qt::CaseInsensitive) ||
            getValue().contains(query, Qt::CaseInsensitive);
}

/**************************/
PwAttachment::PwAttachment(QObject* parent) :
        QObject(parent),
        name(""),
        size(0),
        content(NULL) {

}

PwAttachment::~PwAttachment() {
    // nothing to do here
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
    qint64 size = outFile.write(content->data);
    if (size != content->data.size()) {
        qDebug("%d bytes written out of %d total", (int)size, content->data.size());
        return false;
    }
    outFile.close();
    return true;
}

;void PwAttachment::setName(const QString& name) {
    if (this->name != name) {
        this->name = name;
        emit nameChanged(name);
    }
}

void PwAttachment::setContent(PwBinaryV4* content) {
    // The content is managed by PwDatabaseV4
    this->content = content;
    emit sizeChanged(content->data.size());
}

int PwAttachment::getSize() {
    if (content->isCompressed) {
        bool inflateOk = inflateData();
        if (!inflateOk)
            return -1;
    }
    return content->data.size();
}

bool PwAttachment::inflateData() {
    if (content->isCompressed) {
        QByteArray unpackedData;
        Util::ErrorCode err = Util::inflateGZipData(content->data, unpackedData);
        if (err != Util::SUCCESS) {
            qDebug() << "Attachment inflate error" << err;
            return false;
        }
        content->data = unpackedData;
        content->isCompressed = false;
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

PwEntryV4::PwEntryV4(QObject* parent) :
        PwEntry(parent),
        fields(),
        _extraFieldsDataModel(),
        _historyDataModel(),
        _attachmentsDataModel() {
    _extraFieldsDataModel.setParent(this);
    _historyDataModel.setParent(this);
    _attachmentsDataModel.setParent(this);
}

PwEntryV4::~PwEntryV4() {
    //nothing to do here
}

void PwEntryV4::clear() {
    _historyDataModel.clear();
    _extraFieldsDataModel.clear();
    _attachmentsDataModel.clear();
    fields.clear();
}

bool PwEntryV4::isStandardField(const QString& name) const {
    return (name == TITLE) || (name == USERNAME) || (name == PASSWORD) || (name == URL) || (name == NOTES);
}

/** Search helper. Returns true if any of the fields contain the query string. */
bool PwEntryV4::matchesQuery(const QString& query) const {
    if (PwEntry::matchesQuery(query))
        return true;

    for (int i = 0; i < _extraFieldsDataModel.size(); i++) {
        if (_extraFieldsDataModel.value(i)->matchesQuery(query))
            return true;
    }
    for (int i = 0; i < _attachmentsDataModel.size(); i++) {
        if (_attachmentsDataModel.value(i)->matchesQuery(query))
            return true;
    }
    return false;
}

void PwEntryV4::setField(const QString& name, const QString& value) {
    fields.insert(name, value);
    if (!isStandardField(name)) {
        PwExtraField* ef = new PwExtraField(this, name, value);
        _extraFieldsDataModel.append(ef); // implicitly takes ownership
    }
}

void PwEntryV4::addHistoryEntry(PwEntryV4* historyEntry) {
    _historyDataModel.append(historyEntry); // implicitly takes ownership
}

void PwEntryV4::addAttachment(PwAttachment* attachment) {
    _attachmentsDataModel.append(attachment); // implicitly takes ownership
}

QString PwEntryV4::getTitle() const {
    return fields.value(TITLE);
}
void PwEntryV4::setTitle(const QString& title) {
    setField(TITLE, title);
}
QString PwEntryV4::getUserName() const {
    return fields.value(USERNAME);
}
void PwEntryV4::setUserName(const QString& userName) {
    setField(USERNAME, userName);
}
QString PwEntryV4::getPassword() const {
    return fields.value(PASSWORD);
}
void PwEntryV4::setPassword(const QString& password) {
    setField(PASSWORD, password);
}
QString PwEntryV4::getUrl() const {
    return fields.value(URL);
}
void PwEntryV4::setUrl(const QString& url) {
    setField(URL, url);
}
QString PwEntryV4::getNotes() const {
    return fields.value(NOTES);
}
void PwEntryV4::setNotes(const QString& notes) {
    setField(NOTES, notes);
}
