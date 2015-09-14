/*
 * ApplicationUI.hpp
 *
 *  Created on: 8 May 2014
 *      Author: Andrei Popleteev
 */

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include "util/Util.h"
#include <bb/cascades/Application>
#include <bb/cascades/LocaleHandler>
#include <bb/system/InvokeManager>
#include <bb/device/VibrationController>
#include <QtSensors/QProximitySensor>
#include "db/PwDatabase.h"
#include "util/TimedClipboard.h"
#include "util/Settings.h"
#include "util/PasswordGenerator.h"

class QTranslator;

using namespace QtMobility;

/*!
 * @brief Application object
 *
 *
 */
class ApplicationUI : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool quickLocked READ isQuickLocked NOTIFY quickLockedChanged)
private:
    bb::cascades::AbstractPane* qmlRoot;
    Settings* settings;
    PwDatabaseFacade* database;
    PwGroup* _parentGroup;
    bb::system::InvokeManager* invokeManager;
    TimedClipboard clipboard;
    PasswordGenerator* passwordGenerator;
    QTimer watchdog;
    QByteArray quickPassHash;
    bool quickLocked; // quick lock (i.e. "app lock") state (it is different from DB lock!)
    QProximitySensor sensor;
    bool sensorWasClose;
    bb::device::VibrationController vibrationController;
    bool isMinimized; // true while the app is minimized/thumbnailed
    bool isMultiCopyConfrimed; // have we already notified the user about multi-copy after minimization?

    QTranslator* m_pTranslator;
    bb::cascades::LocaleHandler* m_pLocaleHandler;

    void initQml(bb::cascades::Application *app);

    // property accessors
    bool isQuickLocked() const { return quickLocked; }
    void setQuickLocked(bool newLocked);

    void startMultiCopySensor();
    void stopMultiCopySensor();
public:
    ApplicationUI(bb::cascades::Application *app);
    virtual ~ApplicationUI() { }

    void showToast(const QString& msg);

    // copy given text to the clipboard, clear it after some time
    Q_INVOKABLE void copyWithTimeout(const QString& text);
    // stores entry fields for consequent multi-copying
    Q_INVOKABLE void prepareMultiCopy(const QString& userName, const QString& password);

    /**
     * Opens the given URI (file or web link) with a suitable third-party app.
     * Shows an error toast in case of trouble, unless suppressErrors is true.
     */
    Q_INVOKABLE void invokeFileOrUrl(const QString& uri, bool suppressErrors = false);

    void vibrateBriefly();

    // Checks whether quick unlock code matches
    Q_INVOKABLE bool quickUnlock(const QString& quickCode);
    // Remembers the necessary info for quick unlock
    Q_INVOKABLE void prepareQuickUnlock(const QString& fullPassword);

    Q_INVOKABLE PasswordGenerator* getPasswordGenerator() const;

    /** Checks if the app has permission to access file system */
    Q_INVOKABLE bool canAccessSharedFiles() const;
private slots:
    void onSystemLanguageChanged();
    void onWatchdogTimeoutChanged(int timeout);
    void onThumbnail(); // app minimized
    void onFullscreen(); // app restored
    void onTimeout(); // watchdog timeout
    void onAboutToQuit();
    void onSensorReadingChanged();
    void onClipboardCleared();
public slots:
    Q_INVOKABLE void restartWatchdog();
    Q_INVOKABLE void stopWatchdog();
    void onInvoke(const bb::system::InvokeRequest& request);
    Q_INVOKABLE void lock();
signals:
    void clipboardUpdated();
    void clipboardCleared();
    void dbOpenError(const QString& message, const PwDatabase::ErrorCode errorCode);
    // emitted when the app enters the quick-lock state
    void appLocked();
    // emitted when the app lock (quick lock) state changes
    void quickLockedChanged(bool locked);
    // emitted when the app has been invoked with a valid DB file path
    void invokedWithDatabase(const QString& dbFilePath);
};

#endif /* ApplicationUI_HPP_ */
