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

class Settings: public QObject {
    Q_OBJECT
    /**
     * Flag indicating whether DB search looks into Recycle Bin
     */
    Q_PROPERTY(bool searchInDeleted READ isSearchInDeleted WRITE setSearchInDeleted NOTIFY searchInDeletedChanged)
    /**
     * Time in millis until entries copied to clipboard are deleted from there. Negative means "never".
     */
    Q_PROPERTY(int clipboardTimeout READ getClipboardTimeout WRITE setClipboardTimeout NOTIFY clipboardTimeoutChanged)
    /**
     * Flag indicating whether th last opened DB path should be remembered
     */
    Q_PROPERTY(bool trackRecentDb READ isTrackRecentDb WRITE setTrackRecentDb NOTIFY trackRecentDbChanged);
    /**
     * Time in millis until automatic DB lock. Negative value means no timeout.
     */
    Q_PROPERTY(int autoLockTimeout READ getAutoLockTimeout WRITE setAutoLockTimeout NOTIFY autoLockTimeoutChanged)
    /**
     * Use alphabetical sorting in group lists
     */
    Q_PROPERTY(bool alphaSorting READ isAlphaSorting WRITE setAlphaSorting NOTIFY alphaSortingChanged)
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

private:
    static Settings* _instance;
    bool _searchInDeleted;
    int _clipboardTimeout;
    bool _trackRecentDb;
    int _autoLockTimeout;
    bool _alphaSorting;
    EntryListDetail _entryListDetail;
    bool _quickUnlockEnabled;
    QuickUnlockType _quickUnlockType;
    QStringList _recentFiles;
    QMap<QString, QString> _recentDbToKey;

    void loadRecentFiles();
    void saveRecentFiles();
public:
    /**
     * This constructor should be private, but enums of uncreatable types are not available from QML.
     */
    Settings(QObject* parent = 0);

    /**
     * Returns the singleton instance of Settings
     */
    static Settings* instance();

    // property accessors
    bool isSearchInDeleted() const { return _searchInDeleted; }
    int getClipboardTimeout() const { return _clipboardTimeout; }
    bool isTrackRecentDb() const { return _trackRecentDb; }
    int getAutoLockTimeout() const { return _autoLockTimeout; }
    bool isAlphaSorting() const { return _alphaSorting; }
    EntryListDetail getEntryListDetail() const { return _entryListDetail; }
    bool isQuickUnlockEnabled() const { return _quickUnlockEnabled; }
    QuickUnlockType getQuickUnlockType() const { return _quickUnlockType; }

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
     * Returns the key file path last used for the given database;
     * or empty string if no such file found.
     */
    Q_INVOKABLE QString getKeyFileForDatabase(const QString& dbFile) const;

public slots:
    void setSearchInDeleted(bool searchInDeleted);
    void setClipboardTimeout(int timeout);
    void setTrackRecentDb(bool track);
    void setAutoLockTimeout(int timeout);
    void setAlphaSorting(bool alphaSorting);
    void setEntryListDetail(EntryListDetail detail);
    void setQuickUnlockEnabled(bool enabled);
    void setQuickUnlockType(QuickUnlockType type);
signals:
    void searchInDeletedChanged(bool);
    void clipboardTimeoutChanged(int);
    void trackRecentDbChanged(bool);
    void autoLockTimeoutChanged(int);
    void alphaSortingChanged(bool);
    void entryListDetailChanged(EntryListDetail);
    void quickUnlockEnabledChanged(bool);
    void quickUnlockTypeChanged(QuickUnlockType);
};

#endif /* SETTINGS_H_ */
