/*
 * ApplicationUI.cpp
 *
 *  Created on: 8 May 2014
 *      Author: Andrei Popleteev
 */

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/QmlDocument>
#include <bb/system/SystemToast>
#include <bb/system/InvokeManager>
#include <bb/device/VibrationController>
#include "db/PwDatabase.h"
#include "crypto/CryptoManager.h"
#include "ui/ActiveFrame.h"
#include "util/Util.h"

using namespace bb::cascades;
using namespace bb::system;
using namespace bb::device;

const QString OPEN_DB_ACTION = "org.keepassb.database.open";

ApplicationUI::ApplicationUI(bb::cascades::Application *app) :
        QObject(app), clipboard(app), watchdog(), quickPassHash(), quickLocked(false),
        sensor(app), vibrationController(app), isMinimized(false), isMultiCopyConfrimed(false) {

    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);
    invokeManager = new InvokeManager(this);

    bool res;
    // Since the variable is not used in the app, this is added to avoid a compiler warning
    Q_UNUSED(res);

    res = QObject::connect(&clipboard, SIGNAL(inserted()), this, SIGNAL(clipboardUpdated())); Q_ASSERT(res);
    res = QObject::connect(&clipboard, SIGNAL(cleared()), this, SIGNAL(clipboardCleared())); Q_ASSERT(res);

    res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged())); Q_ASSERT(res);

    // initial load
    onSystemLanguageChanged();

    // init random seed -- for Utils, not for cryptography
    qsrand((uint)QTime::currentTime().msec());

    settings = Settings::instance();
    settings->setParent(app);

    CryptoManager* cryptoManager = CryptoManager::instance();
    cryptoManager->setParent(app);
    if (cryptoManager->init() != SB_SUCCESS) {
        showToast(tr("Cryptographic library error", "Generic error message from a cryptographic library"));
    }

    passwordGenerator = PasswordGenerator::instance();
    passwordGenerator->setParent(app);

    database = new PwDatabaseFacade(this);

    app->setCover(new ActiveFrame(app, this, database));
    res = QObject::connect(app, SIGNAL(thumbnail()), this, SLOT(onThumbnail())); Q_ASSERT(res);
    res = QObject::connect(app, SIGNAL(fullscreen()), this, SLOT(onFullscreen())); Q_ASSERT(res);
    res = QObject::connect(app, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit())); Q_ASSERT(res);


    watchdog.setSingleShot(true);
    watchdog.setInterval(settings->getAutoLockTimeout());
    res = QObject::connect(settings, SIGNAL(autoLockTimeoutChanged(int)), this, SLOT(onWatchdogTimeoutChanged(int))); Q_ASSERT(res);
    res = QObject::connect(&watchdog, SIGNAL(timeout()), this, SLOT(onTimeout())); Q_ASSERT(res);

    // lock DB and clear clipboard when leaving the app (since the timeout timer won't fire after that)
    res = QObject::connect(app, SIGNAL(manualExit()), database, SLOT(lock())); Q_ASSERT(res);
    res = QObject::connect(app, SIGNAL(manualExit()), &clipboard, SLOT(clear())); Q_ASSERT(res);

    sensor.setAlwaysOn(true); // otherwise sensors are inactive even with run_when_backgrounded
    sensor.setSkipDuplicates(true); // to reduce processing (has no effect in 10.3.1 yet)
    res = QObject::connect(&sensor, SIGNAL(readingChanged()), this, SLOT(onSensorReadingChanged())); Q_ASSERT(res);

    initQml(app);
}

void ApplicationUI::initQml(bb::cascades::Application *app) {
    QmlDocument* qml = QmlDocument::create("qrc:/assets/main.qml").parent(this);
    qml->setContextProperty("app", this);
    qml->setContextProperty("appSettings", this->settings);
    qml->setContextProperty("database", this->database);

    // Create root object for the UI
    qmlRoot = qml->createRootObject<AbstractPane>();

    // Set created root object as the application scene
    app->setScene(qmlRoot);
}

// Handles invocation from other apps.
// Expects a database path in request URI.
void ApplicationUI::onInvoke(const InvokeRequest& request) {
    QUrl uri = request.uri();

    if (uri.isEmpty())
        return;

    QString filePath = uri.toLocalFile();
    LOG("filePath: %s", filePath.toUtf8().constData());
    if (!filePath.isEmpty()) {
        emit invokedWithDatabase(filePath);
    }
}

void ApplicationUI::onSystemLanguageChanged() {
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("KeePassB_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

void ApplicationUI::onClipboardCleared() {
    stopMultiCopySensor();
}

void ApplicationUI::onFullscreen() {
    LOG("App restored");
    stopMultiCopySensor();
    isMinimized = false;
}

void ApplicationUI::onThumbnail() {
    // Surprisingly, onThumbnail() gets called also when another app is minimized.
    // So we consider only the first, genuine minimization.
    LOG("onThumbnail: %s", Application::instance()->isThumbnailed() ? "already thumbnailed" : "fullscreen");
    if (isMinimized)
        return;

    LOG("App minimized");
    isMinimized = true;

    if (!database->isLocked()) {
        startMultiCopySensor();
        // zero timeout means lock when minimized
        if (settings->getAutoLockTimeout() == 0)
            lock();
    }
}

void ApplicationUI::onAboutToQuit() {
    LOG("App quitting.");
    clipboard.clear();
    stopMultiCopySensor();
}


void ApplicationUI::startMultiCopySensor() {
    if (clipboard.isPairActive()) {
        sensor.start();
        sensorWasClose = sensor.reading()->close();
        LOG("Sensor started");
    } else {
        LOG("No multi-copy activated");
    }
}

void ApplicationUI::stopMultiCopySensor() {
    sensor.stop();
    LOG("Sensor stopped");
}

void ApplicationUI::vibrateBriefly() {
    vibrationController.stop();
    vibrationController.start(100, 10);
}

void ApplicationUI::onSensorReadingChanged() {
    if (!isMinimized) {
        LOG("Sensor reading while fullscreen - ignoring");
        return;
    }
    if (!isMultiCopyConfrimed) {
        // The app just minimized, confirm that multi copy is activated
        // (Without run_when_backgrounded permission we won't be here)
        vibrateBriefly();
        isMultiCopyConfrimed = true;
        LOG("Multi copy confirmed");
    }

    bool isClose = sensor.reading()->close();
    if (isClose != sensorWasClose) {
        if (isClose) {
            LOG("Multi copy trigger event.");
            // Multi copy remote event just triggered
            vibrateBriefly();
            clipboard.activateAlternative();
            stopMultiCopySensor();
        }
        sensorWasClose = isClose;
        LOG("Got new reading");
    }
}

void ApplicationUI::showToast(const QString& msg) {
    SystemToast* toast = new SystemToast(this);
    toast->setPosition(SystemUiPosition::MiddleCenter);
    toast->setBody(msg);
    toast->show();
}

void ApplicationUI::onWatchdogTimeoutChanged(int timeout) {
    /*
     * Zero timeout is a special case, means "lock DB when app minimized".
     * Negative is "never lock".
     */
    if (timeout <= 0) {
        stopWatchdog();
    } else {
        watchdog.setInterval(timeout);
    }
}

void ApplicationUI::restartWatchdog() {
    // Zero timeout means lock when minimized; negative is "never" -- so we ignore the watchdog timeout
    if (settings->getAutoLockTimeout() > 0)
        watchdog.start();
}

void ApplicationUI::stopWatchdog() {
    watchdog.stop();
}

// copy given text to the clipboard, clear it after some time
void ApplicationUI::copyWithTimeout(const QString& text) {
	clipboard.insertWithTimeout(text, settings->getClipboardTimeout());
	if (settings->isMinimizeAppOnCopy())
	    Application::instance()->minimize();
}

// stores entry fields for consequent multi-copying
void ApplicationUI::prepareMultiCopy(const QString& userName, const QString& password) {
    LOG("prepareMultiCopy");
    clipboard.insertPair(userName, password, settings->getClipboardTimeout());
    isMultiCopyConfrimed = false;
    if (settings->isMinimizeAppOnCopy())
        Application::instance()->minimize();
}

/**
 * Opens the given URI (file or web link) with a suitable third-party app.
 * Shows an error toast in case of trouble, unless suppressErrors is true.
 */
void ApplicationUI::invokeFileOrUrl(const QString& uri, bool suppressErrors) {
    InvokeRequest request;
    request.setUri(uri);
    request.setAction("bb.action.OPEN");
    const InvokeTargetReply* reply = invokeManager->invoke(request);
    if (reply) {
        LOG("invoke ok");
    } else {
        LOG("invoke failed");
        if (!suppressErrors)
            showToast(tr("Cannot open the file", "An error message related to the 'open file' action (reference: INVOKE_ATTACHMENT)"));
    }
}

/**
 * Called on watchdog timeout, possibly with no DB open.
 */
void ApplicationUI::onTimeout() {
    // Timeouts can be emitted even when no DB is opened (and have no effect then).
    // So we show a notification only if something will actually be locked.
    if (!database->isLocked()) {
        showToast(tr("Locked due to inactivity timeout", "Notification shown when the database is locked (or Quick Lock'ed) after some time without user interaction."));
    }
    lock();
}

void ApplicationUI::lock() {
    // lock() might be triggered by the watchdog even before any DB is opened,
    // e.g. when the user reads through the Settings.
    // So if there is nothing to lock, just quit.
    if (database->isLocked())
        return;

    if (settings->isQuickUnlockEnabled()) {
        setQuickLocked(true);
        emit appLocked();
    } else {
        Util::safeClear(quickPassHash);
        database->lock();
        setQuickLocked(false); // either app or DB can be locked, but not both
    }
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

void ApplicationUI::prepareQuickUnlock(const QString& fullPassword) {
    int len = fullPassword.length();

    QString quickPass;
    switch (settings->getQuickUnlockType()) {
    case Settings::QUICK_UNLOCK_FIRST_3:
        quickPass = fullPassword.left(min(3, len));
        break;
    case Settings::QUICK_UNLOCK_FIRST_4:
        quickPass = fullPassword.left(min(4, len));
        break;
    case Settings::QUICK_UNLOCK_FIRST_5:
        quickPass = fullPassword.left(min(5, len));
        break;
    case Settings::QUICK_UNLOCK_LAST_3:
        quickPass = fullPassword.right(min(3, len));
        break;
    case Settings::QUICK_UNLOCK_LAST_4:
        quickPass = fullPassword.right(min(4, len));
        break;
    case Settings::QUICK_UNLOCK_LAST_5:
        quickPass = fullPassword.right(min(5, len));
        break;
    default:
        LOG("Unknown quick unlock type: %d", settings->getQuickUnlockType());
    }
    int err = CryptoManager::instance()->sha256(quickPass.toUtf8(), quickPassHash);
    if (err != SB_SUCCESS) {
        LOG("Prepare quick unlock: hashing error %d", err);
        // log it, but do not bother the user
    }
}

bool ApplicationUI::quickUnlock(const QString& quickPass) {
    // We'll either open the Quick Lock, or lock the app completely.
    // In either case, the app will not be quick-locked anymore.
    setQuickLocked(false);
    if (quickPass.length() == 0)
        return false;

    QByteArray candidateHash;
    int err = CryptoManager::instance()->sha256(quickPass.toUtf8(), candidateHash);
    if (err != SB_SUCCESS) {
        LOG("Quick unlock: hashing error %d", err);
        return false;
    }
    return (candidateHash == quickPassHash);
}

PasswordGenerator* ApplicationUI::getPasswordGenerator() const {
    return passwordGenerator;
}

void ApplicationUI::setQuickLocked(bool newLocked) {
    if (quickLocked != newLocked) {
        quickLocked = newLocked;
        emit quickLockedChanged(quickLocked);
    }
}

/** Checks if the app has permission to access file system */
bool ApplicationUI::canAccessSharedFiles() const {
    bool result = false;
    QDir dir;
    QTemporaryFile testFile(dir.absoluteFilePath("shared/documents/keepassb.XXXXXX"));
    if (testFile.open()) {
        testFile.write("file access test");
        result = true;
        testFile.close();
    }
    LOG("File access permission: %s (%s)",
            (result ? "granted" : "denied"),
            testFile.fileName().toUtf8().constData());
    return result;
}
