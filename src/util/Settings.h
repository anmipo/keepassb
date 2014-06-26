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

private:
    static Settings* _instance;
    bool _searchInDeleted;
    int _clipboardTimeoutIndex;
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
public slots:
    void setSearchInDeleted(bool searchInDeleted);
    void setClipboardTimeoutIndex(int index);
signals:
    void searchInDeletedChanged(bool);
    void clipboardTimeoutIndexChanged(int);
};

#endif /* SETTINGS_H_ */
