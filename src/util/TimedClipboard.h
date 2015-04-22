/*
 * TimedClipboard.h
 *
 *  Created on: 19 May 2014
 *      Author: Andrei Popleteev
 */

#ifndef TIMEDCLIPBOARD_H_
#define TIMEDCLIPBOARD_H_

#include <bb/system/Clipboard>
#include <QTimer>

class TimedClipboard : public bb::system::Clipboard {
	Q_OBJECT
public:
    enum SelectedContentType {
        CONTENT_NONE   = 0x00, // there is no content in app's clipboard
        CONTENT_SIMPLE = 0x01, // current content is a simple string
        CONTENT_FIRST  = 0x02, // current content is the first string of a pair
        CONTENT_SECOND = 0x03  // current content is the second string of a pair
    };

private:
    SelectedContentType selectedContentType;

	static const QString DATA_TYPE; // MIME type of clipboard data we work with
	QTimer timer;
	QByteArray simpleContent;
	QByteArray firstContent;
	QByteArray secondContent;

	bool modified; // did we insert anything to system's clipboard?
private slots:
	void timeout();
public:
	TimedClipboard(QObject* parent);
	virtual ~TimedClipboard();
	// negative timeout means no timeout
	bool insertWithTimeout(const QString& text, const long timeoutMillis);

	void insertPair(const QString& first, const QString& second, const long timeoutMillis);
	void activateFirst();
	void activateSecond();
	void activateAlternative();

	bool isPairActive();

public slots:
    /**
     * Removes the previously inserted content from the clipboard,
     * leaving any other content intact.
     * Returns true if something was removed.
     */
    bool clear();
signals:
    void cleared();
    void inserted();
};

#endif /* TIMEDCLIPBOARD_H_ */
