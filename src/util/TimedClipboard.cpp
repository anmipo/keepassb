/*
 * TimedClipboard.cpp
 *
 *  Created on: 19 May 2014
 *      Author: Andrei Popleteev
 */

#include "TimedClipboard.h"

#include <bb/system/Clipboard>
#include <QObject>
#include "util/Util.h"

const QString TimedClipboard::DATA_TYPE = "text/plain";

TimedClipboard::TimedClipboard(QObject* parent) :
		bb::system::Clipboard(parent), selectedContentType(CONTENT_NONE),
		timer(parent), simpleContent(), firstContent(), secondContent(), modified(false) {
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
    if (!modified)
        return false;

    QByteArray actualClipboardContent = this->value(DATA_TYPE);
    bool containsOurContent;
    switch (selectedContentType) {
        case CONTENT_SIMPLE:
            containsOurContent = (simpleContent == actualClipboardContent);
            break;
        case CONTENT_FIRST:
            containsOurContent = (firstContent == actualClipboardContent);
            break;
        case CONTENT_SECOND:
            containsOurContent = (secondContent == actualClipboardContent);
            break;
        default: // such as CONTENT_NONE
            containsOurContent = false;
    }
    // clear internal state
    Util::safeClear(simpleContent);
    Util::safeClear(firstContent);
    Util::safeClear(secondContent);
    selectedContentType = CONTENT_NONE;

    if (containsOurContent) {
        // remove our data from system's clipboard
        LOG("Clipboard cleared by timeout");
        // Clipboard::clear() leaves the old data in /accounts/1000/clipboard/text.plain
        // so we need to insert something else first (thanks CellNinja!)
        // Inserting an empty string has no effect, though, therefore an arbitrary "0".
        this->insert(DATA_TYPE, "0");
        bool result = bb::system::Clipboard::clear();
        emit cleared();
        return result;
    } else {
        LOG("Clipboard left as is (there is no app's data)");
        return false;
    }
}

bool TimedClipboard::insertWithTimeout(const QString& text, const long timeoutMillis) {
    simpleContent = text.toUtf8();
    // Clearing the complete clipboard is very important, since there could co-exist
    // different texts in various formats: text/plain, text/rtf, text/html...
    bb::system::Clipboard::clear();

	bool result = this->insert(DATA_TYPE, simpleContent);
	if (result) {
        modified = true;
		emit inserted();
		selectedContentType = CONTENT_SIMPLE;
		timer.stop();
		if (timeoutMillis >= 0) {
		    timer.start(timeoutMillis);
		}
	}
	return result;
}

void TimedClipboard::insertPair(const QString& first, const QString& second, const long timeoutMillis) {
    this->firstContent = first.toUtf8();
    this->secondContent = second.toUtf8();

    timer.stop();
    activateFirst();
    if (timeoutMillis >= 0) {
        timer.start(timeoutMillis);
    }
}

void TimedClipboard::activateFirst() {
    // Clearing the complete clipboard is very important, since there could co-exist
    // different texts in various formats: text/plain, text/rtf, text/html...
    bb::system::Clipboard::clear();
    bool result = this->insert(DATA_TYPE, firstContent);
    if (result) {
        LOG("Clipboard content: %s", this->value(DATA_TYPE).constData());
        modified = true;
        emit inserted();
        selectedContentType = CONTENT_FIRST;
    }
}

void TimedClipboard::activateSecond() {
    // Clearing the complete clipboard is very important, since there could co-exist
    // different texts in various formats: text/plain, text/rtf, text/html...
    bb::system::Clipboard::clear();
    bool result = this->insert(DATA_TYPE, secondContent);
    if (result) {
        LOG("Clipboard content: %s", this->value(DATA_TYPE).constData());
        modified = true;
        emit inserted();
        selectedContentType = CONTENT_SECOND;
    }
}

void TimedClipboard::activateAlternative() {
//    LOG("activateAlternative(): selectedContentType is %d", selectedContentType);
    if (selectedContentType == CONTENT_FIRST)
        activateSecond();
    else if (selectedContentType == CONTENT_SECOND) {
        activateFirst();
    } else {
        LOG("Cannot alternate clipboard content - not in multi-copy mode");
    }
}

bool TimedClipboard::isPairActive() {
    return (selectedContentType == CONTENT_FIRST) || (selectedContentType == CONTENT_SECOND);
}
