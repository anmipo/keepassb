/*
 * PwStreamUtilsV4.cpp
 *
 *  Created on: 1 Dec 2014
 *      Author: Andrei
 */

#include <PwStreamUtilsV4.h>
#include <qdebug.h>
#include "util/Util.h"

const QString TRUE_STRING = "True";
const QString FALSE_STRING = "False";

/** Reads a boolean from current XML element's text; in case of errors returns the default value. */
bool PwStreamUtilsV4::readBool(QXmlStreamReader& xml, bool defaultValue) {
    QString valueStr = xml.readElementText();
    if (valueStr.compare(TRUE_STRING, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (valueStr.compare(FALSE_STRING, Qt::CaseInsensitive) == 0) {
        return false;
    } else {
        return defaultValue;
    }
}

/** Reads a long from current XML element; in case of errors returns the default value. */
qint64 PwStreamUtilsV4::readInt64(QXmlStreamReader& xml, qint64 defaultValue) {
    QString valueStr = xml.readElementText();
    bool ok;
    qint64 result = valueStr.toLongLong(&ok);
    if (!ok) {
        result = defaultValue;
    }
    return result;
}

/** Reads an unsigned int from current XML element; in case of errors returns the default value. */
quint32 PwStreamUtilsV4::readUInt32(QXmlStreamReader& xml, quint32 defaultValue) {
    QString valueStr = xml.readElementText();
    bool ok;
    quint32 result = valueStr.toUInt(&ok);
    if (!ok) {
        result = defaultValue;
    }
    return result;
}

/** Reads a signed int from current XML element; in case of errors returns the default value. */
qint32 PwStreamUtilsV4::readInt32(QXmlStreamReader& xml, qint32 defaultValue) {
    QString valueStr = xml.readElementText();
    bool ok;
    qint32 result = valueStr.toInt(&ok);
    if (!ok) {
        result = defaultValue;
    }
    return result;
}

/** Reads a string from current XML element. */
QString PwStreamUtilsV4::readString(QXmlStreamReader& xml) {
    return xml.readElementText();
}

/** Reads a UTC timestamp from current XML element; in case of error returns current time. */
QDateTime PwStreamUtilsV4::readTime(QXmlStreamReader& xml) {
    QString valueStr = xml.readElementText();
    QDateTime result = QDateTime::fromString(valueStr, Qt::ISODate);
    if (!result.isValid()) {
        result = QDateTime::currentDateTime();
    }
    return result;
}

/** Reads a Base-64 encoded UUID from current XML element; in case of error returns in case of error returns default (all-zero) UUID. */
PwUuid PwStreamUtilsV4::readUuid(QXmlStreamReader& xml) {
    QString base64Str = xml.readElementText();
    PwUuid result = PwUuid::fromBase64(base64Str);
    Util::safeClear(base64Str);
    return result;
}

/** Reads a Base-64 encoded data array from current XML element; in case of error returns an empty array. */
QByteArray PwStreamUtilsV4::readBase64(QXmlStreamReader& xml) {
    QByteArray base64Str = xml.readElementText().toLatin1();
    QByteArray result = QByteArray::fromBase64(base64Str);
    Util::safeClear(base64Str);
    return result;
}

/** Skips to the end tag of an unknown element, ignoring any nested elements. */
void PwStreamUtilsV4::readUnknown(QXmlStreamReader& xml) {
    QStringRef unknownTagName = xml.name();
    qDebug() << "WARN: unknown XML element: " << unknownTagName;

    while (!xml.hasError() && !(xml.isEndElement() && (xml.name() == unknownTagName))) {
        xml.readNext();
        if (xml.isStartElement()) {
            readUnknown(xml);
        }
    }
}
