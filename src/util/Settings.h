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
     * Index of the selected "Clipboard timeout" option in the settings page.
     * See also: getClipboardTimeoutSeconds() returns the corresponding time.
     */
    Q_PROPERTY(int clipboardTimeoutIndex READ getClipboardTimeoutIndex WRITE setClipboardTimeoutIndex NOTIFY clipboardTimeoutIndexChanged)
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
private:
    static Settings* _instance;
    bool _searchInDeleted;
    int _clipboardTimeoutIndex;
    bool _trackRecentDb;
    QString _recentDbPath;
    QString _recentKeyFilePath;
    Settings(QObject* parent = 0);

    // Matching from index to actual timeout values
    static const int CLIPBOARD_TIMEOUT_INDEX_TO_SECONDS[];
public:
    /**
     * Returns the singleton instance of Settings
     */
    static Settings* instance();

    /**
     * Time in seconds until entries copied to clipboard are deleted from there
     */
    int getClipboardTimeoutSeconds() const;

    // property accessors
    bool isSearchInDeleted() const { return _searchInDeleted; }
    int getClipboardTimeoutIndex() const { return _clipboardTimeoutIndex; }
    bool isTrackRecentDb() const { return _trackRecentDb; }
    QString getRecentDbPath() const { return _recentDbPath; }
    QString getRecentKeyFilePath() const { return _recentKeyFilePath; }
public slots:
    void setSearchInDeleted(bool searchInDeleted);
    void setClipboardTimeoutIndex(int index);
    void setTrackRecentDb(bool track);
    void setRecentDbPath(const QString& path);
    void setRecentKeyFilePath(const QString& path);
signals:
    void searchInDeletedChanged(bool);
    void clipboardTimeoutIndexChanged(int);
    void trackRecentDbChanged(bool);
    void recentDbPathChanged(QString);
    void recentKeyFilePathChanged(QString);
};

#endif /* SETTINGS_H_ */
