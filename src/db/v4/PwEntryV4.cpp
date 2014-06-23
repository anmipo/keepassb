/*
 * PwEntryV4.cpp
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#include "db/v4/PwEntryV4.h"

// Standard entry fields
const static QString TITLE = "Title";
const static QString USERNAME = "UserName";
const static QString PASSWORD = "Password";
const static QString URL = "URL";
const static QString NOTES = "Notes";


PwExtraField::PwExtraField() : QObject() {
    _name = "";
    _value = "";
}

PwExtraField::PwExtraField(const QString& name, const QString& value) : QObject() {
    _name = name;
    _value = value;
}

/**************************/

PwEntryV4::PwEntryV4() :
        PwEntry(),
        fields(),
        _extraFieldsDataModel(),
        _historyDataModel() {
    _extraFieldsDataModel.setParent(this);
    _historyDataModel.setParent(this);
}

PwEntryV4::~PwEntryV4() {
    //nothing to do here
}

void PwEntryV4::clear() {
    _historyDataModel.clear();
    _extraFieldsDataModel.clear();
    fields.clear();
}

bool PwEntryV4::isStandardField(const QString& name) const {
    return (name == TITLE) || (name == USERNAME) || (name == PASSWORD) || (name == URL) || (name == NOTES);
}

void PwEntryV4::setField(const QString& name, const QString& value) {
    fields.insert(name, value);
    if (!isStandardField(name)) {
        PwExtraField* ef = new PwExtraField(name, value);
        _extraFieldsDataModel.append(ef); // implicitly takes ownership
    }
}

void PwEntryV4::addHistoryEntry(PwEntryV4* historyEntry) {
    _historyDataModel.append(historyEntry); // implicitly takes ownership
}

bool PwEntryV4::matchesQuery(const QString& query) const {
    //TODO consider extra fields
    return getTitle().contains(query, Qt::CaseInsensitive) ||
           getUserName().contains(query, Qt::CaseInsensitive) ||
           getUrl().contains(query, Qt::CaseInsensitive) ||
           getNotes().contains(query, Qt::CaseInsensitive);
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
