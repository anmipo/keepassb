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
private:
	static const QString DATA_TYPE; // MIME type of clipboard data we work with
	QTimer timer;
public slots:
	void timeout();
signals:
	void cleared();
	void inserted();
public:
	TimedClipboard(QObject* parent);
	virtual ~TimedClipboard();
	// negative timeout means no timeout
	bool insertWithTimeout(const QString& text, const long timeoutMillis);
};

#endif /* TIMEDCLIPBOARD_H_ */
