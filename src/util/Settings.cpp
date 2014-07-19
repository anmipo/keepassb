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
const bool DEFAULT_QUICK_UNLOCK_ENABLED = false;
const int DEFAULT_QUICK_UNLOCK_TYPE = Settings::QUICK_UNLOCK_FIRST_4;

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
const QString KEY_QUICK_UNLOCK_ENABLED = "quickUnlockEnabled";
const QString KEY_QUICK_UNLOCK_TYPE = "quickUnlockType";

Settings* Settings::_instance;

Settings* Settings::instance() {
    if (!_instance) {
        _instance = new Settings();
    }
    return _instance;
}

Settings::Settings(QObject* parent) : QObject(parent){
    // This should have been qmlRegisterUncreatableType(), but then
    // contained enums would not be accessible from QML.
    // So I had to make the constructor public and register type as creatable....
    qmlRegisterType<Settings>("org.keepassb", 1, 0, "Settings");

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
    _entryListDetail = (EntryListDetail)settings.value(
            KEY_ENTRY_LIST_DETAIL, DEFAULT_ENTRY_LIST_DETAIL).toInt();
    _quickUnlockEnabled = settings.value(
            KEY_QUICK_UNLOCK_ENABLED, DEFAULT_QUICK_UNLOCK_ENABLED).toBool();
    _quickUnlockType = (QuickUnlockType) settings.value(
            KEY_QUICK_UNLOCK_TYPE, DEFAULT_QUICK_UNLOCK_TYPE).toInt();
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

void Settings::setEntryListDetail(EntryListDetail detail) {
    if (detail != _entryListDetail) {
        QSettings().setValue(KEY_ENTRY_LIST_DETAIL, detail);
        _entryListDetail = detail;
        emit entryListDetailChanged(detail);
    }
}

void Settings::setQuickUnlockEnabled(bool enabled) {
    if (enabled != _quickUnlockEnabled) {
        QSettings().setValue(KEY_QUICK_UNLOCK_ENABLED, enabled);
        _quickUnlockEnabled = enabled;
        emit quickUnlockEnabledChanged(enabled);
    }
}

void Settings::setQuickUnlockType(Settings::QuickUnlockType type) {
    if (type != _quickUnlockType) {
        QSettings().setValue(KEY_QUICK_UNLOCK_TYPE, type);
        _quickUnlockType = type;
        emit quickUnlockTypeChanged(type);
    }
}
