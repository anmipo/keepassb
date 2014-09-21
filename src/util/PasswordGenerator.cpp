/*
 * PasswordGenerator.cpp
 *
 *  Created on: 21 Sep 2014
 *      Author: Andrei
 */

#include <PasswordGenerator.h>
#include "crypto/CryptoManager.h"
#include <bb/cascades/Application>

const QString CHAR_SET_LOWER  = "abcdefghijklmnopqrstuvwxyz";
const QString CHAR_SET_UPPER  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const QString CHAR_SET_DIGITS  = "0123456789";
const QString CHAR_SET_SPECIAL = "`~!@#$%^&*_+()[]{}<>\\|:;,.?/'\"";
const QString CHAR_SET_LOOKALIKE = "Il|10O";

PasswordGenerator* PasswordGenerator::_instance;

PasswordGenerator* PasswordGenerator::instance() {
    if (!_instance) {
        _instance = new PasswordGenerator();
    }
    return _instance;
}

PasswordGenerator::PasswordGenerator(QObject* parent) : QObject(parent) {
    qmlRegisterUncreatableType<PasswordGenerator>("org.keepassb", 1, 0, "PasswordGenerator", "PasswordGenerator is a singleton");
}

/**
 * Returns a random string made of the allowed characters.
 * If all parameters are false, returns an empty string.
 */
QString PasswordGenerator::makeCustomPassword(const int length, const bool lowerCase, const bool upperCase, const bool numbers, const bool specials, const bool excludeSimilar) const {
    QString charset;
    if (lowerCase)
        charset += CHAR_SET_LOWER;
    if (upperCase)
        charset += CHAR_SET_UPPER;
    if (numbers)
        charset += CHAR_SET_DIGITS;
    if (specials)
        charset += CHAR_SET_SPECIAL;
    if (excludeSimilar) {
        for (int i = 0; i < CHAR_SET_LOOKALIKE.length(); i++) {
            charset.remove(CHAR_SET_LOOKALIKE.at(i), Qt::CaseSensitive);
        }
    }

    int charsetSize = charset.size();
    if (charsetSize == 0) {
        qDebug() << "password charset is empty";
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
