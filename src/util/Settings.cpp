/*
 * Settings.cpp
 *
 *  Created on: 26 Jun 2014
 *      Author: Andrei Popleteev
 */

#include "util/Settings.h"
#include <bb/cascades/Application>
#include <QCoreApplication>
#include <QSettings>


/**
 * Default settings values
 */
const bool DEFAULT_SEARCH_IN_DELETED = false;
const int DEFAULT_CLIPBOARD_TIMEOUT = 10 * 1000;
const bool DEFAULT_TRACK_RECENT_DB = true;
const QString DEFAULT_RECENT_DB_PATH = "";
const QString DEFAULT_RECENT_KEY_FILE_PATH = "";
const int DEFAULT_AUTO_LOCK_TIMEOUT = 60 * 1000;
const bool DEFAULT_ALPHA_SORTING = false;
const int DEFAULT_ENTRY_LIST_DETAIL = Settings::ENTRY_DETAIL_USER_NAME;

/**
 * Keys for preferences values
 */
const QString KEY_SEARCH_IN_DELETED = "searchInDeleted";
const QString KEY_CLIPBOARD_TIMEOUT = "clipboardTimeout";
const QString KEY_TRACK_RECENT_DB = "trackRecentDb";
const QString KEY_RECENT_DB_PATH = "recentDbPath";
const QString KEY_RECENT_KEY_FILE_PATH = "recentKeyFilePath";
const QString KEY_AUTO_LOCK_TIMEOUT = "autoLockTimeout";
const QString KEY_ALPHA_SORTING = "alphaSorting";
const QString KEY_ENTRY_LIST_DETAIL = "entryListDetail";

Settings* Settings::_instance;

Settings* Settings::instance() {
    if (!_instance) {
        _instance = new Settings();
    }
    return _instance;
}

Settings::Settings(QObject* parent) : QObject(parent){
    qmlRegisterUncreatableType<Settings>("org.keepassb", 1, 0, "Settings", "Settings is a singleton with a private constructor");

    // Set up the QSettings object for the application with organization and application name.
    QCoreApplication::setOrganizationName("Andrei Popleteev");
    QCoreApplication::setOrganizationDomain("keepassb.org");
    QCoreApplication::setApplicationName("KeePassB");

    QSettings settings;
    _searchInDeleted = settings.value(
            KEY_SEARCH_IN_DELETED, DEFAULT_SEARCH_IN_DELETED).toBool();
    _clipboardTimeout = settings.value(
            KEY_CLIPBOARD_TIMEOUT, DEFAULT_CLIPBOARD_TIMEOUT).toInt();
    _trackRecentDb = settings.value(
            KEY_TRACK_RECENT_DB, DEFAULT_TRACK_RECENT_DB).toBool();
    _recentDbPath = settings.value(
            KEY_RECENT_DB_PATH, DEFAULT_RECENT_DB_PATH).toString();
    _recentKeyFilePath = settings.value(
            KEY_RECENT_KEY_FILE_PATH, DEFAULT_RECENT_KEY_FILE_PATH).toString();
    _autoLockTimeout = settings.value(
            KEY_AUTO_LOCK_TIMEOUT, DEFAULT_AUTO_LOCK_TIMEOUT).toInt();
    _alphaSorting = settings.value(
            KEY_ALPHA_SORTING, DEFAULT_ALPHA_SORTING).toBool();
    _entryListDetail = settings.value(
            KEY_ENTRY_LIST_DETAIL, DEFAULT_ENTRY_LIST_DETAIL).toInt();
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

void Settings::setTrackRecentDb(bool track) {
    if (track != _trackRecentDb) {
        QSettings().setValue(KEY_TRACK_RECENT_DB, track);
        _trackRecentDb = track;
        emit trackRecentDbChanged(track);
    }
}

void Settings::setRecentDbPath(const QString& path) {
    if (path != _recentDbPath) {
        QSettings().setValue(KEY_RECENT_DB_PATH, path);
        _recentDbPath = path;
        emit recentDbPathChanged(path);
    }
}

void Settings::setRecentKeyFilePath(const QString& path) {
    if (path != _recentKeyFilePath) {
        QSettings().setValue(KEY_RECENT_KEY_FILE_PATH, path);
        _recentKeyFilePath = path;
        emit recentKeyFilePathChanged(path);
    }
}

void Settings::setAutoLockTimeout(int timeout) {
    if (timeout != _autoLockTimeout) {
        QSettings().setValue(KEY_AUTO_LOCK_TIMEOUT, timeout);
        _autoLockTimeout = timeout;
        emit autoLockTimeoutChanged(timeout);
    }
}

void Settings::setAlphaSorting(bool alphaSorting) {
    if (alphaSorting != _alphaSorting) {
        QSettings().setValue(KEY_ALPHA_SORTING, alphaSorting);
        _alphaSorting = alphaSorting;
        emit alphaSortingChanged(alphaSorting);
    }
}

void Settings::setEntryListDetail(int fieldType) {
    if (fieldType != _entryListDetail) {
        QSettings().setValue(KEY_ENTRY_LIST_DETAIL, fieldType);
        _entryListDetail = fieldType;
        emit entryListDetailChanged(fieldType);
    }
}
