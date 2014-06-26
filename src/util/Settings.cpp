/*
 * Settings.cpp
 *
 *  Created on: 26 Jun 2014
 *      Author: Andrei Popleteev
 */

#include "util/Settings.h"
#include <QCoreApplication>
#include <QSettings>

/**
 * Default settings values
 */
const bool DEFAULT_SEARCH_IN_DELETED(false);
const int DEFAULT_CLIPBOARD_TIMEOUT_INDEX(1);

/**
 * Keys for preferences values
 */
const QString KEY_SEARCH_IN_DELETED = "searchInDeleted";
const QString KEY_CLIPBOARD_TIMEOUT_INDEX = "clipboardTimeoutIndex";

// this must be in sync with options in AppSettings.qml
const int Settings::CLIPBOARD_TIMEOUT_INDEX_TO_SECONDS[] = {-1, 10, 30, 60};

Settings* Settings::_instance;

Settings* Settings::instance() {
    if (!_instance) {
        _instance = new Settings();
    }
    return _instance;
}

Settings::Settings(QObject* parent) : QObject(parent){
    // Set up the QSettings object for the application with organization and application name.
    QCoreApplication::setOrganizationName("Andrei Popleteev");
    QCoreApplication::setOrganizationDomain("keepassb.org");
    QCoreApplication::setApplicationName("KeePassB");

    _searchInDeleted = QSettings().value(
            KEY_SEARCH_IN_DELETED, DEFAULT_SEARCH_IN_DELETED).toBool();
    _clipboardTimeoutIndex = QSettings().value(
            KEY_CLIPBOARD_TIMEOUT_INDEX, DEFAULT_CLIPBOARD_TIMEOUT_INDEX).toInt();
}

void Settings::setSearchInDeleted(bool searchInDeleted) {
    if (searchInDeleted != _searchInDeleted) {
        QSettings().setValue(KEY_SEARCH_IN_DELETED, searchInDeleted);
        _searchInDeleted = searchInDeleted;
        emit searchInDeletedChanged(searchInDeleted);
    }
}

void Settings::setClipboardTimeoutIndex(int index) {
    if (index != _clipboardTimeoutIndex) {
        QSettings().setValue(KEY_CLIPBOARD_TIMEOUT_INDEX, index);
        _clipboardTimeoutIndex = index;
        emit clipboardTimeoutIndexChanged(index);
    }
}

int Settings::getClipboardTimeoutSeconds() const {
    return CLIPBOARD_TIMEOUT_INDEX_TO_SECONDS[_clipboardTimeoutIndex];
}
