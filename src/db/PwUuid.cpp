/*
 * PwUuid.cpp
 *
 *  Created on: 25 Jun 2014
 *      Author: Andrei
 */

#include <PwUuid.h>

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
    bytes.clear();
}

PwUuid PwUuid::fromBase64(const QString& base64) {
    PwUuid uuid(QByteArray::fromBase64(base64.toLatin1()));
    return uuid;
}
