/*
 * TimedClipboard.cpp
 *
 *  Created on: 19 May 2014
 *      Author: Andrei Popleteev
 */

#include "TimedClipboard.h"

#include <bb/system/Clipboard>
#include <QObject>

const QString TimedClipboard::DATA_TYPE = "text/plain";

TimedClipboard::TimedClipboard(QObject* parent) :
		bb::system::Clipboard(parent), timer(parent), content(), modified(false) {
	timer.setSingleShot(true);
	QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

TimedClipboard::~TimedClipboard() {
	timer.stop();
	clear();
}

void TimedClipboard::timeout() {
    timer.stop();
    clear();
}

bool TimedClipboard::clear() {
    if (modified && (content == this->value(DATA_TYPE))) {
        qDebug("Clipboard cleared by timeout");
        bool result = bb::system::Clipboard::clear();
        emit cleared();
        return result;
    } else {
        qDebug("Clipboard NOT cleared by timeout - different content");
        return false;
    }
}

bool TimedClipboard::insertWithTimeout(const QString& text, const long timeoutMillis) {
    content = text.toUtf8();
    modified = true;
    // Clearing the complete clipboard is very important, since there could co-exist
    // different texts in various formats: text/plain, text/rtf, text/html...
    bb::system::Clipboard::clear();

	bool result = this->insert(DATA_TYPE, content);
	if (result) {
		emit inserted();
		if (timeoutMillis >= 0) {
		    timer.start(timeoutMillis);
		}
	}
	return result;
}

