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
		bb::system::Clipboard(parent), timer(parent), content() {
	timer.setSingleShot(true);
	QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

TimedClipboard::~TimedClipboard() {
	timer.stop();
}

void TimedClipboard::timeout() {
	if (content == this->value(DATA_TYPE)) {
        qDebug("Clipboard cleared by timeout");
	    this->remove(DATA_TYPE);
	    emit cleared();
	} else {
        qDebug("Clipboard NOT cleared by timeout - different content");
	}
}

bool TimedClipboard::insertWithTimeout(const QString& text, const long timeoutMillis) {
    content = text.toUtf8();
	bool result = this->insert(DATA_TYPE, content);
	if (result) {
		emit inserted();
		if (timeoutMillis >= 0) {
		    timer.start(timeoutMillis);
		}
	}
	return result;
}

