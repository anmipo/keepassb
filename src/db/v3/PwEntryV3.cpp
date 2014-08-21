/*
 * PwEntryV3.cpp
 *
 *  Created on: 21 Aug 2014
 *      Author: Andrei
 */

#include <PwEntryV3.h>

PwEntryV3::PwEntryV3(QObject* parent) : PwEntry(parent) {
    // TODO Auto-generated constructor stub
}

PwEntryV3::~PwEntryV3() {
    clear();
}

void PwEntryV3::setGroupId(qint32 groupId) {
    if (groupId != _groupId) {
        groupId = _groupId;
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
