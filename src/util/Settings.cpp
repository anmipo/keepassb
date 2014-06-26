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
const int DEFAULT_CLIPBOARD_TIMEOUT(15);

/**
 * Keys for preferences values
 */
const QString KEY_SEARCH_IN_DELETED = "searchInDeleted";
const QString KEY_CLIPBOARD_TIMEOUT = "clipboardTimeout";


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
    _clipboardTimeout = QSettings().value(
            KEY_CLIPBOARD_TIMEOUT, DEFAULT_CLIPBOARD_TIMEOUT).toInt();
}

void Settings::setSearchInDeleted(bool searchInDeleted) {
    if (searchInDeleted != _searchInDeleted) {
        QSettings().setValue(KEY_SEARCH_IN_DELETED, searchInDeleted);
        _searchInDeleted = searchInDeleted;
        emit searchInDeletedChanged(searchInDeleted);
    }
}

void Settings::setClipboardTimeout(int timeout) {
    if (timeout != _clipboardTimeout) {
        QSettings().setValue(KEY_CLIPBOARD_TIMEOUT, timeout);
        _clipboardTimeout = timeout;
        emit clipboardTimeoutChanged(timeout);
    }
}
