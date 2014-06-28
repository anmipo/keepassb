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
const bool DEFAULT_SEARCH_IN_DELETED = false;
const int DEFAULT_CLIPBOARD_TIMEOUT_INDEX = 1;
const bool DEFAULT_TRACK_RECENT_DB = false;
const QString DEFAULT_RECENT_DB_PATH = "";
const QString DEFAULT_RECENT_KEY_FILE_PATH = "";

/**
 * Keys for preferences values
 */
const QString KEY_SEARCH_IN_DELETED = "searchInDeleted";
const QString KEY_CLIPBOARD_TIMEOUT_INDEX = "clipboardTimeoutIndex";
const QString KEY_TRACK_RECENT_DB = "trackRecentDb";
const QString KEY_RECENT_DB_PATH = "recentDbPath";
const QString KEY_RECENT_KEY_FILE_PATH = "recentKeyFilePath";

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

    QSettings settings;
    _searchInDeleted = settings.value(
            KEY_SEARCH_IN_DELETED, DEFAULT_SEARCH_IN_DELETED).toBool();
    _clipboardTimeoutIndex = settings.value(
            KEY_CLIPBOARD_TIMEOUT_INDEX, DEFAULT_CLIPBOARD_TIMEOUT_INDEX).toInt();
    _trackRecentDb = settings.value(
            KEY_TRACK_RECENT_DB, DEFAULT_TRACK_RECENT_DB).toBool();
    _recentDbPath = settings.value(
            KEY_RECENT_DB_PATH, DEFAULT_RECENT_DB_PATH).toString();
    _recentKeyFilePath = settings.value(
            KEY_RECENT_KEY_FILE_PATH, DEFAULT_RECENT_KEY_FILE_PATH).toString();
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
