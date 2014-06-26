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
     * Time in seconds until entries copied to clipboard are deleted from there
     */
    Q_PROPERTY(int clipboardTimeout READ getClipboardTimeout WRITE setClipboardTimeout NOTIFY clipboardTimeoutChanged)

private:
    bool _searchInDeleted;
    int _clipboardTimeout;
public:
    Settings(QObject* parent = 0);

    // property accessors
    bool isSearchInDeleted() const { return _searchInDeleted; }
    int getClipboardTimeout() const { return _clipboardTimeout; }
public slots:
    void setSearchInDeleted(bool searchInDeleted);
    void setClipboardTimeout(int timeout);
signals:
    void searchInDeletedChanged(bool);
    void clipboardTimeoutChanged(int);
};

#endif /* SETTINGS_H_ */
