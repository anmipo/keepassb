/*
 * PasswordGenerator.cpp
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#include <PasswordGenerator.h>
#include <bb/cascades/Application>
#include "crypto/CryptoManager.h"
#include "util/Settings.h"
#include "util/Util.h"

const QString CHAR_SET_LOWER  = "abcdefghijklmnopqrstuvwxyz";
const QString CHAR_SET_UPPER  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const QString CHAR_SET_DIGITS  = "0123456789";
const QString CHAR_SET_SPECIAL = "`~!@#$%^&*_+()[]{}<>\\|:;,.?/'\"";
const QString CHAR_SET_LOOKALIKE = "Il|10O";

PasswordGenerator* PasswordGenerator::_instance;

PasswordGenerator* PasswordGenerator::instance() {
    if (!_instance) {
        _instance = new PasswordGenerator(QApplication::instance());
    }
    return _instance;
}

PasswordGenerator::PasswordGenerator(QObject* parent) : QObject(parent) {
    qmlRegisterUncreatableType<PasswordGenerator>("org.keepassb", 1, 0, "PasswordGenerator", "PasswordGenerator is a singleton");
}

PasswordGenerator::~PasswordGenerator() {
    _instance = NULL;
}

/**
 * Returns a random string made of the allowed characters.
 * Flags define included and excluded character sets (see Settings::PWGEN_INCLUDE_* and Setting::PWGEN_EXCLUDE_*)
 * If no charset included, returns an empty string.
 */
QString PasswordGenerator::makeCustomPassword(const int length, const int flags) const {
    QString charset;
    if (flags & PWGEN_INCLUDE_LOWER)
        charset += CHAR_SET_LOWER;
    if (flags & PWGEN_INCLUDE_UPPER)
        charset += CHAR_SET_UPPER;
    if (flags & PWGEN_INCLUDE_DIGITS)
        charset += CHAR_SET_DIGITS;
    if (flags & PWGEN_INCLUDE_SPECIALS)
        charset += CHAR_SET_SPECIAL;
    if (flags & PWGEN_EXCLUDE_SIMILAR) {
        for (int i = 0; i < CHAR_SET_LOOKALIKE.length(); i++) {
            charset.remove(CHAR_SET_LOOKALIKE.at(i), Qt::CaseSensitive);
        }
    }

    int charsetSize = charset.size();
    if (charsetSize == 0) {
        LOG("password charset is empty");
        return QString("");
    }

    QString result;
    result.reserve(length);

    CryptoManager* cm = CryptoManager::instance();
    QByteArray bytes;
    cm->getRandomBytes(bytes, length);
    for (int i = 0; i < bytes.size(); i++) {
        int pos = (bytes.at(i) & 0x000000FF) % charsetSize;
        result += charset.at(pos);
    }
    return result;
}

/**
 * Returns a string containing nBytes of random data in hex representation.
 */
QString PasswordGenerator::makeHexPassword(const int nBytes) const {
    CryptoManager* cm = CryptoManager::instance();
    QByteArray bytes;
    cm->getRandomBytes(bytes, nBytes);

    return QString(bytes.toHex());
}

/**
 * Returns a random MAC address (format: HH-HH-HH-HH-HH-HH)
 */
QString PasswordGenerator::makeMacAddress() const {
    CryptoManager* cm = CryptoManager::instance();
    QByteArray bytes;
    cm->getRandomBytes(bytes, 6);

    QStringList groups;
    for (int i = 0; i < bytes.size(); i++) {
        QString group = QString::number(bytes.at(i) & 0x000000FF, 16);
        if (group.length() == 1)
            group.prepend("0");
        groups.append(group.toUpper());
    }
    return groups.join("-");
}
