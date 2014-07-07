/*
 * ApplicationUI.hpp
 *
 *  Created on: 8 May 2014
 *      Author: Andrei Popleteev
 */

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include <bb/cascades/Application>
#include <bb/cascades/LocaleHandler>
#include <bb/system/InvokeManager>
#include "db/PwDatabase.h"
#include "util/TimedClipboard.h"
#include "util/Settings.h"

class QTranslator;


/*!
 * @brief Application object
 *
 *
 */
class ApplicationUI : public QObject
{
    Q_OBJECT
private:
    bb::cascades::AbstractPane* qmlRoot;
    Settings* settings;
    PwDatabaseFacade* database;
    PwGroup* _parentGroup;
    bb::system::InvokeManager* invokeManager;
    TimedClipboard clipboard;
    QTimer watchdog;

    QTranslator* m_pTranslator;
    bb::cascades::LocaleHandler* m_pLocaleHandler;

    void initQml(bb::cascades::Application *app);
public:
    ApplicationUI(bb::cascades::Application *app);
    virtual ~ApplicationUI() { }

    // copy given text to the clipboard, clear it after some time
    Q_INVOKABLE void copyWithTimeout(const QString& text);
    void showToast(const QString& msg);
    // opens given file with a suitable third-party app
    Q_INVOKABLE void invokeFile(const QString& uri);
private slots:
    void onSystemLanguageChanged();
    void onWatchdogTimeoutChanged(int timeout);
    void onThumbnail(); // app minimized
public slots:
    Q_INVOKABLE void restartWatchdog();
    Q_INVOKABLE void stopWatchdog();
    void onInvoke(const bb::system::InvokeRequest& request);
signals:
    void clipboardUpdated();
    void clipboardCleared();
    void dbOpenError(const QString& message, const PwDatabase::Error errorCode);
};

#endif /* ApplicationUI_HPP_ */
