/*
 * PwUuid.cpp
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#include <PwUuid.h>
#include <QUuid.h>
#include "util/Util.h"

const int UUID_SIZE = 16;

PwUuid::PwUuid() : bytes(UUID_SIZE, 0) {
    // left empty
}

PwUuid::PwUuid(const QByteArray& aBytes) {
    bytes = Util::deepCopy(aBytes);
}

PwUuid::~PwUuid() {
    clear();
}

/** Resets the UUID to all-zero bytes */
void PwUuid::clear() {
    bytes.fill(0, UUID_SIZE);
}

/** Generates and returns a new UUID instance. */
PwUuid PwUuid::create() {
    // QUuid uses cryptographic-quality RNG when available.
    QByteArray uuidBytes = QUuid::createUuid().toRfc4122();
    return PwUuid(uuidBytes);
}

PwUuid PwUuid::fromBase64(const QString& base64) {
    QByteArray bytes = QByteArray::fromBase64(base64.toLatin1());
    if (bytes.size() != UUID_SIZE) {
        LOG("PwUuid::fromBase64 - wrong UUID size: %d '%s' (Base64: %s)",
                bytes.size(), bytes.toHex().constData(), base64.toUtf8().constData());
    }

    if (bytes.isEmpty())
        return PwUuid(); // 16 zeroes
    else
        return PwUuid(bytes);
}
