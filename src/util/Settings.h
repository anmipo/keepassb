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
     * Previously opened database file path
     */
    Q_PROPERTY(QString recentDbPath READ getRecentDbPath WRITE setRecentDbPath NOTIFY recentDbPathChanged);
    /**
     * Previously used key file path
     */
    Q_PROPERTY(QString recentKeyFilePath READ getRecentKeyFilePath WRITE setRecentKeyFilePath NOTIFY recentKeyFilePathChanged);
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
    Q_PROPERTY(int entryListDetail READ getEntryListDetail WRITE setEntryListDetail NOTIFY entryListDetailChanged)

private:
    static Settings* _instance;
    bool _searchInDeleted;
    int _clipboardTimeout;
    bool _trackRecentDb;
    QString _recentDbPath;
    QString _recentKeyFilePath;
    int _autoLockTimeout;
    bool _alphaSorting;
    int _entryListDetail;

    Settings(QObject* parent = 0);

public:
    enum EntryListDetail {
        ENTRY_DETAIL_NONE      = 0,
        ENTRY_DETAIL_USER_NAME = 1,
        ENTRY_DETAIL_PASSWORD  = 2,
        ENTRY_DETAIL_URL       = 4,
        ENTRY_DETAIL_NOTES     = 8,
        ENTRY_DETAIL_LAST_MODIFIED_TIME = 16
        // eventual additions should be powers of 2: 2, 4, 8, ...
    };
    Q_ENUMS(EntryListDetail);

    /**
     * Returns the singleton instance of Settings
     */
    static Settings* instance();

    // property accessors
    bool isSearchInDeleted() const { return _searchInDeleted; }
    int getClipboardTimeout() const { return _clipboardTimeout; }
    bool isTrackRecentDb() const { return _trackRecentDb; }
    QString getRecentDbPath() const { return _recentDbPath; }
    QString getRecentKeyFilePath() const { return _recentKeyFilePath; }
    int getAutoLockTimeout() const { return _autoLockTimeout; }
    bool isAlphaSorting() const { return _alphaSorting; }
    int getEntryListDetail() const { return _entryListDetail; }
public slots:
    void setSearchInDeleted(bool searchInDeleted);
    void setClipboardTimeout(int timeout);
    void setTrackRecentDb(bool track);
    void setRecentDbPath(const QString& path);
    void setRecentKeyFilePath(const QString& path);
    void setAutoLockTimeout(int timeout);
    void setAlphaSorting(bool alphaSorting);
    void setEntryListDetail(int fieldType);
signals:
    void searchInDeletedChanged(bool);
    void clipboardTimeoutChanged(int);
    void trackRecentDbChanged(bool);
    void recentDbPathChanged(QString);
    void recentKeyFilePathChanged(QString);
    void autoLockTimeoutChanged(int);
    void alphaSortingChanged(bool);
    void entryListDetailChanged(int);
};

#endif /* SETTINGS_H_ */
