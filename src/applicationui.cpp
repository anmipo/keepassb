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
#include <bb/system/SystemToast>
#include <bb/system/InvokeManager>
#include "db/PwDatabase.h"
#include "crypto/CryptoManager.h"
#include "ui/ActiveFrame.h"

using namespace bb::cascades;
using namespace bb::system;

const QString OPEN_DB_ACTION = "org.keepassb.database.open";

ApplicationUI::ApplicationUI(bb::cascades::Application *app) :
        QObject(app), clipboard(app), watchdog() {

    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);
    invokeManager = new InvokeManager(this);

    Q_ASSERT(QObject::connect(&clipboard, SIGNAL(inserted()), this, SIGNAL(clipboardUpdated())));
    Q_ASSERT(QObject::connect(&clipboard, SIGNAL(cleared()), this, SIGNAL(clipboardCleared())));

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    // This is only available in Debug builds
    Q_ASSERT(res);
    // Since the variable is not used in the app, this is added to avoid a
    // compiler warning
    Q_UNUSED(res);

    // initial load
    onSystemLanguageChanged();

    settings = Settings::instance();
    settings->setParent(app);

    CryptoManager::instance()->init();

    Application::instance()->setCover(new ActiveFrame());
    Q_ASSERT(QObject::connect(Application::instance(), SIGNAL(thumbnail()), this, SLOT(onThumbnail())));

    database = new PwDatabaseFacade();
    database->setParent(this);

    watchdog.setSingleShot(true);
    watchdog.setInterval(settings->getAutoLockTimeout());
    Q_ASSERT(QObject::connect(settings, SIGNAL(autoLockTimeoutChanged(int)), this, SLOT(onWatchdogTimeoutChanged(int))));
    Q_ASSERT(QObject::connect(&watchdog, SIGNAL(timeout()), database, SLOT(lock())));

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
        // write the invocation path to the "Database path" property of the "Unlock DB" page
        QObject* unlockDbPage = qmlRoot->findChild<QObject*>("unlockDbPage");
        QDeclarativeProperty(unlockDbPage, "dbFilePath").write(filePath);
    }
}

void ApplicationUI::onSystemLanguageChanged() {
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("KPB_proto1_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

void ApplicationUI::onThumbnail() {
    qDebug() << "app minimized";

    // zero timeout means lock when minimized
    if (!database->isLocked() && settings->getAutoLockTimeout() == 0) {
        database->lock();
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
     * Zero timeout is a special case, means "lock DB when app minimized"
     */
    if (timeout == 0) {
        stopWatchdog();
    } else {
        watchdog.setInterval(timeout);
    }
}

void ApplicationUI::restartWatchdog() {
    watchdog.start();
}
void ApplicationUI::stopWatchdog() {
    watchdog.stop();
}

// copy given text to the clipboard, clear it after some time
void ApplicationUI::copyWithTimeout(const QString& text) {
	clipboard.insertWithTimeout(text, settings->getClipboardTimeout());
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
        showToast(tr("Invoke failed"));
    }
}
