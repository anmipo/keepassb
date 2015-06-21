/*
 * Settings.h
 *
 *  Provides access to application settings.
 *
 *  Created on: 26 Jun 2014
 *      Author: Andrei Popleteev
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QStringList>
#include "util/PasswordGenerator.h"

class Settings: public QObject {
    Q_OBJECT
    /**
     * Flag indicating whether DB search looks into Recycle Bin
     */
    Q_PROPERTY(bool searchInDeleted READ isSearchInDeleted WRITE setSearchInDeleted NOTIFY searchInDeletedChanged)
    /**
     * Should we switch to search mode after unlocking a DB?
     */
    Q_PROPERTY(bool searchAfterUnlock READ isSearchAfterUnlock WRITE setSearchAfterUnlock NOTIFY searchAfterUnlockChanged)
    /**
     * Time in millis until entries copied to clipboard are deleted from there. Negative means "never".
     */
    Q_PROPERTY(int clipboardTimeout READ getClipboardTimeout WRITE setClipboardTimeout NOTIFY clipboardTimeoutChanged)
    /**
     * What kind of recently used files should we keep in history?
     */
    Q_PROPERTY(TrackRecentFilesType trackRecentFiles READ getTrackRecentFiles WRITE setTrackRecentFiles NOTIFY trackRecentFilesChanged);
    /**
     * Time in millis until automatic DB lock. Negative value means no timeout.
     */
    Q_PROPERTY(int autoLockTimeout READ getAutoLockTimeout WRITE setAutoLockTimeout NOTIFY autoLockTimeoutChanged)
    /**
     * Type of group/entry list sorting (which field, ascending/descending)
     */
    Q_PROPERTY(GroupSortingType groupSortingType READ getGroupSortingType WRITE setGroupSortingType NOTIFY groupSortingTypeChanged)
    /**
     * Type of entry field to show in the description of an entry item in group list
     */
    Q_PROPERTY(EntryListDetail entryListDetail READ getEntryListDetail WRITE setEntryListDetail NOTIFY entryListDetailChanged)
    /**
     * Last state of the "Enable quick unlock" checkbox
     */
    Q_PROPERTY(bool quickUnlockEnabled READ isQuickUnlockEnabled WRITE setQuickUnlockEnabled NOTIFY quickUnlockEnabledChanged)
    /**
     * Type of quick unlock password, specifying which part of the full password is used and its size
     */
    Q_PROPERTY(QuickUnlockType quickUnlockType READ getQuickUnlockType WRITE setQuickUnlockType NOTIFY quickUnlockTypeChanged)
    /**
     * Password generator preset profile
     */
    Q_PROPERTY(PasswordGenerator::PwGenPreset pwGenPreset READ getPwGenPreset WRITE setPwGenPreset NOTIFY pwGenPresetChanged)
    /**
     * Length of generated passwords
     */
    Q_PROPERTY(int pwGenLength READ getPwGenLength WRITE setPwGenLength NOTIFY pwGenLengthChanged)
    /**
     * Password generation features (includes and excluded character sets)
     */
    Q_PROPERTY(int pwGenFlags READ getPwGenFlags WRITE setPwGenFlags NOTIFY pwGenFlagsChanged)
    /**
     * Backup original database on save.
     */
    Q_PROPERTY(bool backupDatabaseOnSave READ isBackupDatabaseOnSave WRITE setBackupDatabaseOnSave NOTIFY backupDatabaseOnSaveChanged)
    /**
     * Minimize application on copy to clipboard.
     */
    Q_PROPERTY(bool minimizeAppOnCopy READ isMinimizeAppOnCopy WRITE setMinimizeAppOnCopy NOTIFY minimizeAppOnCopyChanged)
public:
    enum EntryListDetail {
        ENTRY_DETAIL_NONE      = 0x00,
        ENTRY_DETAIL_USER_NAME = 0x01,
        ENTRY_DETAIL_PASSWORD  = 0x02,
        ENTRY_DETAIL_URL       = 0x04,
        ENTRY_DETAIL_NOTES     = 0x08,
        ENTRY_DETAIL_LAST_MODIFIED_TIME = 0x10
        // eventual additions should be powers of 2: 2, 4, 8, ...
    };
    Q_ENUMS(EntryListDetail);
    enum QuickUnlockType {
        QUICK_UNLOCK_FIRST_3   = 0x03,
        QUICK_UNLOCK_FIRST_4   = 0x04,
        QUICK_UNLOCK_FIRST_5   = 0x05,
        QUICK_UNLOCK_LAST_3    = 0x13,
        QUICK_UNLOCK_LAST_4    = 0x14,
        QUICK_UNLOCK_LAST_5    = 0x15
    };
    Q_ENUMS(QuickUnlockType);
    enum TrackRecentFilesType {
        TRACK_RECENT_FILES_NONE       = 0x00,
        TRACK_RECENT_FILES_DB_ONLY    = 0x01,
        TRACK_RECENT_FILES_DB_AND_KEY = 0x03,
    };
    Q_ENUMS(TrackRecentFilesType);
    enum GroupSortingType {
        GROUP_SORTING_NONE                        = 0x00,
        GROUP_SORTING_NAME_ASC                    = 0x10,
        GROUP_SORTING_NAME_DESC                   = 0x11,
        GROUP_SORTING_CREATION_TIME_ASC           = 0x20,
        GROUP_SORTING_CREATION_TIME_DESC          = 0x21,
        GROUP_SORTING_LAST_MODIFICATION_TIME_ASC  = 0x30,
        GROUP_SORTING_LAST_MODIFICATION_TIME_DESC = 0x31,
        // GROUP_SORTING_LAST_ACCESS_TIME       = 0x40, -- this timestamp updates only on save, so would look inconsistent
    };
    Q_ENUMS(GroupSortingType);
private:
    static Settings* _instance;
    bool _searchInDeleted;
    bool _searchAfterUnlock;
    int _clipboardTimeout;
    TrackRecentFilesType _trackRecentFiles;
    int _autoLockTimeout;
    GroupSortingType _groupSortingType;
    EntryListDetail _entryListDetail;
    bool _quickUnlockEnabled;
    QuickUnlockType _quickUnlockType;
    PasswordGenerator::PwGenPreset _pwGenPreset;
    int _pwGenLength;
    int _pwGenFlags;
    bool _backupDatabaseOnSave;
    bool _minimizeAppOnCopy;
    QStringList _recentFiles;
    QMap<QString, QString> _recentDbToKey;

    void loadRecentFiles();
    void saveRecentFiles();
public:
    /**
     * This constructor should be private, but enums of uncreatable types are not available from QML.
     */
    Settings(QObject* parent = 0);
    virtual ~Settings();

    /**
     * Returns the singleton instance of Settings
     */
    static Settings* instance();

    // property accessors
    bool isSearchInDeleted() const { return _searchInDeleted; }
    bool isSearchAfterUnlock() const { return _searchAfterUnlock; }
    int getClipboardTimeout() const { return _clipboardTimeout; }
    TrackRecentFilesType getTrackRecentFiles() const { return _trackRecentFiles; }
    int getAutoLockTimeout() const { return _autoLockTimeout; }
    GroupSortingType getGroupSortingType() const { return _groupSortingType; }
    EntryListDetail getEntryListDetail() const { return _entryListDetail; }
    bool isQuickUnlockEnabled() const { return _quickUnlockEnabled; }
    QuickUnlockType getQuickUnlockType() const { return _quickUnlockType; }
    PasswordGenerator::PwGenPreset getPwGenPreset() const { return _pwGenPreset; }
    int getPwGenLength() const { return _pwGenLength; }
    int getPwGenFlags() const { return _pwGenFlags; }
    bool isBackupDatabaseOnSave() const { return _backupDatabaseOnSave; }
    bool isMinimizeAppOnCopy() const { return _minimizeAppOnCopy; }

    /**
     * Adds paths to the top of the recent files list
     */
    Q_INVOKABLE void addRecentFiles(const QString& dbFile, const QString& keyFile);
    /**
     * Returns a list of recent database-key paths.
     * Each list item contains both paths separated by "|" (pipe)
     */
    Q_INVOKABLE QStringList getRecentFiles() const;
    /**
     * Clears the list of recent files (permanently).
     */
    Q_INVOKABLE void clearRecentFiles();
    /**
     * Returns the key file path last used for the given database;
     * or empty string if no such file found.
     */
    Q_INVOKABLE QString getKeyFileForDatabase(const QString& dbFile) const;

public slots:
    void setSearchInDeleted(bool searchInDeleted);
    void setSearchAfterUnlock(bool searchAfterUnlock);
    void setClipboardTimeout(int timeout);
    void setTrackRecentFiles(TrackRecentFilesType trackingType);
    void setAutoLockTimeout(int timeout);
    void setGroupSortingType(GroupSortingType sortingType);
    void setEntryListDetail(EntryListDetail detail);
    void setQuickUnlockEnabled(bool enabled);
    void setQuickUnlockType(QuickUnlockType type);
    void setPwGenPreset(PasswordGenerator::PwGenPreset preset);
    void setPwGenLength(int length);
    void setPwGenFlags(int flags);
    void setBackupDatabaseOnSave(bool doBackup);
    void setMinimizeAppOnCopy(bool minOnCopy);
signals:
    void searchInDeletedChanged(bool);
    void searchAfterUnlockChanged(bool);
    void clipboardTimeoutChanged(int);
    void trackRecentFilesChanged(TrackRecentFilesType);
    void autoLockTimeoutChanged(int);
    void groupSortingTypeChanged(GroupSortingType);
    void entryListDetailChanged(EntryListDetail);
    void quickUnlockEnabledChanged(bool);
    void quickUnlockTypeChanged(QuickUnlockType);
    void pwGenPresetChanged(PasswordGenerator::PwGenPreset);
    void pwGenLengthChanged(int);
    void pwGenFlagsChanged(int);
    void backupDatabaseOnSaveChanged(bool);
    void minimizeAppOnCopyChanged(bool);
    // emitted whenever the list of recent DB or key files changes
    void recentFilesChanged();
};

#endif /* SETTINGS_H_ */
