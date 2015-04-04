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
#include "db/PwDatabase.h"
#include "crypto/CryptoManager.h"
#include "ui/ActiveFrame.h"
#include "util/Util.h"

using namespace bb::cascades;
using namespace bb::system;

const QString OPEN_DB_ACTION = "org.keepassb.database.open";

ApplicationUI::ApplicationUI(bb::cascades::Application *app) :
        QObject(app), clipboard(app), watchdog(), quickPassHash()  {

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

    CryptoManager::instance()->init();

    passwordGenerator = PasswordGenerator::instance();
    passwordGenerator->setParent(app);

    database = new PwDatabaseFacade(this);

    app->setCover(new ActiveFrame(app, database));
    res = QObject::connect(app, SIGNAL(thumbnail()), this, SLOT(onThumbnail())); Q_ASSERT(res);

    watchdog.setSingleShot(true);
    watchdog.setInterval(settings->getAutoLockTimeout());
    res = QObject::connect(settings, SIGNAL(autoLockTimeoutChanged(int)), this, SLOT(onWatchdogTimeoutChanged(int))); Q_ASSERT(res);
    res = QObject::connect(&watchdog, SIGNAL(timeout()), this, SLOT(lock())); Q_ASSERT(res);

    // clear clipboard when leaving the app (since the timeout timer won't fire after that)
    res = QObject::connect(app, SIGNAL(manualExit()), &clipboard, SLOT(clear())); Q_ASSERT(res);

    initQml(app);
}

void ApplicationUI::initQml(bb::cascades::Application *app) {
    QmlDocument* qml = QmlDocument::create("asset:///main.qml").parent(this);
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
    qDebug() << "filePath: " << filePath;
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

void ApplicationUI::onThumbnail() {
    qDebug() << "app minimized";

    // zero timeout means lock when minimized
    if (!database->isLocked() && settings->getAutoLockTimeout() == 0) {
        lock();
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
	if (settings->isMinimizeAppOnCopy()) {
	    bool res = Application::instance()->minimize();
	    qDebug() << "App minimize result: " << res;
	}
}

void ApplicationUI::invokeFile(const QString& uri) {
    InvokeRequest request;
    request.setUri(uri);
    request.setAction("bb.action.OPEN");
    const InvokeTargetReply* reply = invokeManager->invoke(request);
    if (reply) {
        qDebug() << "invoke ok";
    } else {
        qDebug() << "invoke failed";
        showToast(tr("Cannot open the file", "An error message related to the 'open file' action (reference: INVOKE_ATTACHMENT)"));
    }
}

void ApplicationUI::lock() {
    // lock() might be triggered by the watchdog even before any DB is opened,
    // e.g. when the user reads through the Settings.
    // So if there is nothing to lock, just quit.
    if (database->isLocked())
        return;

    if (settings->isQuickUnlockEnabled()) {
        emit appLocked();
    } else {
        Util::safeClear(quickPassHash);
        database->lock();
    }
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

void ApplicationUI::prepareQuickUnlock(const QString& fullPassword) {
    // implement this
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
        qDebug() << "Unknown quick unlock type: " << settings->getQuickUnlockType();
    }
    int err = CryptoManager::instance()->sha256(quickPass.toUtf8(), quickPassHash);
    if (err != SB_SUCCESS) {
        qDebug() << "Prepare quick unlock: hashing error " << err;
        // log it, but do not bother the user
    }
}

bool ApplicationUI::quickUnlock(const QString& quickPass) {
    if (quickPass.length() == 0)
        return false;

    QByteArray candidateHash;
    int err = CryptoManager::instance()->sha256(quickPass.toUtf8(), candidateHash);
    if (err != SB_SUCCESS) {
        qDebug() << "Quick unlock: hashing error " << err;
        return false;
    }
    return (candidateHash == quickPassHash);
}

PasswordGenerator* ApplicationUI::getPasswordGenerator() const {
    return passwordGenerator;
}
