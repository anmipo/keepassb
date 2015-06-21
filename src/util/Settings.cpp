/*
 * Settings.cpp
 *
 *  Created on: 26 Jun 2014
 *      Author: Andrei Popleteev
 */

#include "util/Settings.h"
#include "util/Util.h"
#include <bb/cascades/Application>
#include <QCoreApplication>
#include <QSettings>


/**
 * Default settings values
 */
const bool DEFAULT_SEARCH_IN_DELETED = false;
const bool DEFAULT_SEARCH_AFTER_UNLOCK = false;
const int  DEFAULT_CLIPBOARD_TIMEOUT = 30 * 1000;
const int DEFAULT_TRACK_RECENT_FILES = Settings::TRACK_RECENT_FILES_DB_AND_KEY;
const int  DEFAULT_AUTO_LOCK_TIMEOUT = 60 * 1000;
const bool DEFAULT_ALPHA_SORTING = false;
const int DEFAULT_GROUP_SORTING_TYPE = Settings::GROUP_SORTING_NONE;
const int  DEFAULT_ENTRY_LIST_DETAIL = Settings::ENTRY_DETAIL_USER_NAME;
const bool DEFAULT_QUICK_UNLOCK_ENABLED = false;
const int DEFAULT_QUICK_UNLOCK_TYPE = Settings::QUICK_UNLOCK_FIRST_4;
const int DEFAULT_PWGEN_PRESET = PasswordGenerator::PWGEN_PRESET_DEFAULT;
const int DEFAULT_PWGEN_LENGTH = 20;
const int DEFAULT_PWGEN_FLAGS =
        PasswordGenerator::PWGEN_INCLUDE_LOWER |
        PasswordGenerator::PWGEN_INCLUDE_UPPER |
        PasswordGenerator::PWGEN_INCLUDE_DIGITS |
        PasswordGenerator::PWGEN_INCLUDE_SPECIALS;
const bool DEFAULT_BACKUP_DATABASE_ON_SAVE = true;
const bool DEFAULT_MINIMIZE_APP_ON_COPY = true;

/**
 * Keys for preferences values
 */
const QString KEY_SEARCH_IN_DELETED = "searchInDeleted";
const QString KEY_SEARCH_AFTER_UNLOCK = "searchAfterUnlock";
const QString KEY_CLIPBOARD_TIMEOUT = "clipboardTimeout";
const QString KEY_TRACK_RECENT_FILES = "trackRecentFiles";
const QString KEY_AUTO_LOCK_TIMEOUT = "autoLockTimeout";
const QString KEY_ALPHA_SORTING = "alphaSorting";
const QString KEY_GROUP_SORTING_TYPE = "groupSortingType";
const QString KEY_ENTRY_LIST_DETAIL = "entryListDetail";
const QString KEY_QUICK_UNLOCK_ENABLED = "quickUnlockEnabled";
const QString KEY_QUICK_UNLOCK_TYPE = "quickUnlockType";
const QString KEY_PWGEN_PRESET = "pwGenPreset";
const QString KEY_PWGEN_LENGTH = "pwGenLength";
const QString KEY_PWGEN_FLAGS = "pwGenFlags";
const QString KEY_BACKUP_DATABASE_ON_SAVE = "backupDatabaseOnSave";
const QString KEY_MINIMIZE_APP_ON_COPY = "minimizeAppOnCopy";
const QString KEY_RECENT_FILES_COUNT = "recentFiles/count";
const QString KEY_RECENT_FILES_ITEM = "recentFiles/item%1";

const QString RECENT_ITEMS_SEPARATOR = "|";
const int MAX_RECENT_ITEMS_COUNT = 5;

Settings* Settings::_instance;

Settings* Settings::instance() {
    if (!_instance) {
        _instance = new Settings(QApplication::instance());
    }
    return _instance;
}

Settings::Settings(QObject* parent) : QObject(parent) {

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
    _searchAfterUnlock = settings.value(
            KEY_SEARCH_AFTER_UNLOCK, DEFAULT_SEARCH_AFTER_UNLOCK).toBool();
    _clipboardTimeout = settings.value(
            KEY_CLIPBOARD_TIMEOUT, DEFAULT_CLIPBOARD_TIMEOUT).toInt();
    _trackRecentFiles = (TrackRecentFilesType)settings.value(
            KEY_TRACK_RECENT_FILES, DEFAULT_TRACK_RECENT_FILES).toInt();
    _autoLockTimeout = settings.value(
            KEY_AUTO_LOCK_TIMEOUT, DEFAULT_AUTO_LOCK_TIMEOUT).toInt();
    _entryListDetail = (EntryListDetail)settings.value(
            KEY_ENTRY_LIST_DETAIL, DEFAULT_ENTRY_LIST_DETAIL).toInt();
    _quickUnlockEnabled = settings.value(
            KEY_QUICK_UNLOCK_ENABLED, DEFAULT_QUICK_UNLOCK_ENABLED).toBool();
    _quickUnlockType = (QuickUnlockType) settings.value(
            KEY_QUICK_UNLOCK_TYPE, DEFAULT_QUICK_UNLOCK_TYPE).toInt();
    _pwGenPreset = (PasswordGenerator::PwGenPreset) settings.value(
            KEY_PWGEN_PRESET, DEFAULT_PWGEN_PRESET).toInt();
    _pwGenLength = settings.value(
            KEY_PWGEN_LENGTH, DEFAULT_PWGEN_LENGTH).toInt();
    _pwGenFlags = settings.value(
            KEY_PWGEN_FLAGS, DEFAULT_PWGEN_FLAGS).toInt();
    _backupDatabaseOnSave = settings.value(
            KEY_BACKUP_DATABASE_ON_SAVE, DEFAULT_BACKUP_DATABASE_ON_SAVE).toBool();
    _minimizeAppOnCopy = settings.value(
            KEY_MINIMIZE_APP_ON_COPY, DEFAULT_MINIMIZE_APP_ON_COPY).toBool();

    // group sorting type is introduced since v2.4.4, import previous setting
    if (settings.contains(KEY_GROUP_SORTING_TYPE)) {
        _groupSortingType = (GroupSortingType)settings.value(
                KEY_GROUP_SORTING_TYPE, DEFAULT_GROUP_SORTING_TYPE).toInt();
    } else {
        bool alphaSorting = settings.value(
                KEY_ALPHA_SORTING, DEFAULT_ALPHA_SORTING).toBool();
        _groupSortingType = alphaSorting ? GROUP_SORTING_NAME_ASC : GROUP_SORTING_NONE;
    }

    loadRecentFiles();
}

Settings::~Settings() {
    _instance = NULL;
}

void Settings::loadRecentFiles() {
    QSettings settings;

    _recentFiles.clear();
    _recentDbToKey.clear();
    if (_trackRecentFiles == TRACK_RECENT_FILES_NONE) {
        emit recentFilesChanged();
        return;
    }

    int count = settings.value(KEY_RECENT_FILES_COUNT, 0).toInt();
    for (int i = 0; i < count; i++) {
        QString recentItem = settings.value(KEY_RECENT_FILES_ITEM.arg(i)).toString();
        QStringList parts = recentItem.split(RECENT_ITEMS_SEPARATOR);
        _recentFiles.append(parts[0]);
        if (_trackRecentFiles == TRACK_RECENT_FILES_DB_AND_KEY) {
            _recentDbToKey.insert(parts[0], parts[1]);
        }
    }
    emit recentFilesChanged();
}

void Settings::saveRecentFiles() {
    QSettings settings;
    int count = _recentFiles.size();
    if (count > MAX_RECENT_ITEMS_COUNT)
        count = MAX_RECENT_ITEMS_COUNT;
    if (_trackRecentFiles == TRACK_RECENT_FILES_NONE)
        count = 0;
    settings.setValue(KEY_RECENT_FILES_COUNT, count);

    bool trackKeyFiles = (_trackRecentFiles == TRACK_RECENT_FILES_DB_AND_KEY);
    for (int i = 0; i < count; i++) {
        QString dbFile = _recentFiles.at(i);
        QString keyFile = (trackKeyFiles ? _recentDbToKey.value(dbFile, "") : "");
        settings.setValue(KEY_RECENT_FILES_ITEM.arg(i), dbFile + RECENT_ITEMS_SEPARATOR + keyFile);
    }
}

void Settings::addRecentFiles(const QString& dbFile, const QString& keyFile) {
    if (_trackRecentFiles != TRACK_RECENT_FILES_NONE) {
        _recentFiles.removeOne(dbFile); // to avoid duplicates
        _recentFiles.insert(0, dbFile);
        if (_trackRecentFiles == TRACK_RECENT_FILES_DB_AND_KEY)
            _recentDbToKey.insert(dbFile, keyFile);
        saveRecentFiles();
    }
    emit recentFilesChanged();
}

QStringList Settings::getRecentFiles() const {
    QStringList res;
    for (int i = 0 ; i < _recentFiles.size(); i++) {
        QString dbFile = _recentFiles.at(i);
        QString keyFile = _recentDbToKey.value(dbFile, "");
        res.append(dbFile + RECENT_ITEMS_SEPARATOR + keyFile);
    }
    return res;
}

Q_INVOKABLE void Settings::clearRecentFiles() {
    _recentFiles.clear();
    _recentDbToKey.clear();
    emit recentFilesChanged();
    saveRecentFiles();
}

QString Settings::getKeyFileForDatabase(const QString& dbFile) const {
    if (_recentDbToKey.contains(dbFile)) {
        return _recentDbToKey.value(dbFile);
    } else {
        return "";
    }
}

void Settings::setSearchInDeleted(bool searchInDeleted) {
    if (searchInDeleted != _searchInDeleted) {
        QSettings().setValue(KEY_SEARCH_IN_DELETED, searchInDeleted);
        _searchInDeleted = searchInDeleted;
        emit searchInDeletedChanged(searchInDeleted);
    }
}

void Settings::setSearchAfterUnlock(bool searchAfterUnlock) {
    if (searchAfterUnlock != _searchAfterUnlock) {
        QSettings().setValue(KEY_SEARCH_AFTER_UNLOCK, searchAfterUnlock);
        _searchAfterUnlock = searchAfterUnlock;
        emit searchAfterUnlockChanged(searchAfterUnlock);
    }
}

void Settings::setClipboardTimeout(int timeout) {
    if (timeout != _clipboardTimeout) {
        QSettings().setValue(KEY_CLIPBOARD_TIMEOUT, timeout);
        _clipboardTimeout = timeout;
        emit clipboardTimeoutChanged(timeout);
    }
}

void Settings::setTrackRecentFiles(TrackRecentFilesType trackingType) {
    if (trackingType != _trackRecentFiles) {
        QSettings().setValue(KEY_TRACK_RECENT_FILES, trackingType);
        _trackRecentFiles = trackingType;
        emit trackRecentFilesChanged(trackingType);

        // apply change immediately (no other chances later)
        loadRecentFiles();
        saveRecentFiles();
    }
}

void Settings::setAutoLockTimeout(int timeout) {
    if (timeout != _autoLockTimeout) {
        QSettings().setValue(KEY_AUTO_LOCK_TIMEOUT, timeout);
        _autoLockTimeout = timeout;
        emit autoLockTimeoutChanged(timeout);
    }
}

void Settings::setGroupSortingType(GroupSortingType sortingType) {
    if (sortingType != _groupSortingType) {
        QSettings().setValue(KEY_GROUP_SORTING_TYPE, sortingType);
        _groupSortingType = sortingType;
        emit groupSortingTypeChanged(sortingType);
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

void Settings::setPwGenPreset(PasswordGenerator::PwGenPreset preset) {
    if (preset != _pwGenPreset) {
        QSettings().setValue(KEY_PWGEN_PRESET, preset);
        _pwGenPreset = preset;
        emit pwGenPresetChanged(preset);
    }
}

void Settings::setPwGenLength(int length) {
    if (length != _pwGenLength) {
        QSettings().setValue(KEY_PWGEN_LENGTH, length);
        _pwGenLength = length;
        emit pwGenLengthChanged(length);
    }
}

void Settings::setPwGenFlags(int flags) {
    if (flags!= _pwGenFlags) {
        QSettings().setValue(KEY_PWGEN_FLAGS, flags);
        _pwGenFlags = flags;
        emit pwGenFlagsChanged(flags);
    }
}

void Settings::setBackupDatabaseOnSave(bool doBackup) {
    if (doBackup != _backupDatabaseOnSave) {
        QSettings().setValue(KEY_BACKUP_DATABASE_ON_SAVE, doBackup);
        _backupDatabaseOnSave = doBackup;
        emit backupDatabaseOnSaveChanged(doBackup);
    }
}

void Settings::setMinimizeAppOnCopy(bool minOnCopy) {
    if (minOnCopy != _minimizeAppOnCopy) {
        QSettings().setValue(KEY_MINIMIZE_APP_ON_COPY, minOnCopy);
        _minimizeAppOnCopy = minOnCopy;
        emit minimizeAppOnCopyChanged(minOnCopy);
    }
}
