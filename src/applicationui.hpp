/*
 * ApplicationUI.hpp
 *
 *  Created on: 8 May 2014
 *      Author: Andrei Popleteev
 */

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include "db/PwDatabase.h"
#include "util/TimedClipboard.h"
#include "util/Settings.h"

namespace bb
{
    namespace cascades
    {
        class Application;
        class LocaleHandler;
    }
}

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
    Settings* settings;
    PwDatabaseFacade* database;
    PwGroup* _parentGroup;
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
private slots:
    void onSystemLanguageChanged();
    void onWatchdogTimeoutChanged(int timeout);
    void onThumbnail(); // app minimized
public slots:
    Q_INVOKABLE void restartWatchdog();
    Q_INVOKABLE void stopWatchdog();
signals:
    void clipboardUpdated();
    void clipboardCleared();
    void dbOpenError(const QString& message, const PwDatabase::Error errorCode);
};

#endif /* ApplicationUI_HPP_ */
