/*
 * PwUuid.cpp
 *
 *  Created on: 25 Jun 2014
 *      Author: Andrei
 */

#include <PwUuid.h>
#include <QUuid.h>
#include "util/Util.h"

const int UUID_SIZE = 16;

PwUuid::PwUuid() : bytes(UUID_SIZE, 0) {
    // left empty
}

PwUuid::PwUuid(const QByteArray& aBytes) {
    bytes = aBytes;
}

PwUuid::~PwUuid() {
    clear();
}

void PwUuid::clear() {
    Util::safeClear(bytes);
}

/** Generates and returns a new UUID instance. */
PwUuid PwUuid::create() {
    // QUuid uses cryptographic-quality RNG when available.
    QByteArray uuidBytes = QUuid::createUuid().toRfc4122();
    return PwUuid(uuidBytes);
}

PwUuid PwUuid::fromBase64(const QString& base64) {
    PwUuid uuid(QByteArray::fromBase64(base64.toLatin1()));
    return uuid;
}
