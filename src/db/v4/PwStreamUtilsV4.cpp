/*
 * PwStreamUtilsV4.cpp
 *
 *  Created on: 1 Dec 2014
 *      Author: Andrei
 */

#include <PwStreamUtilsV4.h>
#include <qdebug.h>
#include "db/v4/DefsV4.h"
#include "util/Util.h"

/** Reads a boolean from current XML element's text; in case of errors returns the default value. */
bool PwStreamUtilsV4::readBool(QXmlStreamReader& xml, bool defaultValue) {
    QString valueStr = xml.readElementText();
    if (valueStr.compare(XML_TRUE, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (valueStr.compare(XML_FALSE, Qt::CaseInsensitive) == 0) {
        return false;
    } else {
        return defaultValue;
    }
}

/** Writes <tagName>bool_value_as_string</tagName> to the XML stream. */
void PwStreamUtilsV4::writeBool(QXmlStreamWriter& xml, const QString& tagName, const bool value) {
    xml.writeTextElement(tagName, value ? XML_TRUE : XML_FALSE);
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

/** Writes <tagName>value</tagName> to the XML stream. */
void PwStreamUtilsV4::writeInt64(QXmlStreamWriter& xml, const QString& tagName, const qint64 value) {
    xml.writeTextElement(tagName, QString::number(value));
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

/** Writes <tagName>value</tagName> to the XML stream. */
void PwStreamUtilsV4::writeUInt32(QXmlStreamWriter& xml, const QString& tagName, const quint32 value) {
    xml.writeTextElement(tagName, QString::number(value));
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

/** Writes <tagName>value</tagName> to the XML stream. */
void PwStreamUtilsV4::writeInt32(QXmlStreamWriter& xml, const QString& tagName, const qint32 value) {
    xml.writeTextElement(tagName, QString::number(value));
}

/** Reads a string from current XML element. */
QString PwStreamUtilsV4::readString(QXmlStreamReader& xml) {
    return xml.readElementText();
}

/** Writes <tagName>text</tagName> to the XML stream. If text is empty, writes an empty <tagName/>. */
void PwStreamUtilsV4::writeString(QXmlStreamWriter& xml, const QString& tagName, const QString& text) {
    if (text.isEmpty()) {
        xml.writeEmptyElement(tagName);
    } else {
        xml.writeTextElement(tagName, text);
    }
}

/**
 * Reads a UTC timestamp from current XML element; in case of error returns current time.
 * If okPtr is specified, it will set to indicate the success of the conversion.
 */
QDateTime PwStreamUtilsV4::readTime(QXmlStreamReader& xml, bool *okPtr) {
    QString valueStr = xml.readElementText();
    QDateTime result = QDateTime::fromString(valueStr, Qt::ISODate);

    if (okPtr) {
        *okPtr = result.isValid();
    }

    if (!result.isValid()) {
        result = QDateTime::currentDateTime();
    }
    return result;
}

/** Writes <tagName>time_value</tagName> to the XML stream. */
void PwStreamUtilsV4::writeTime(QXmlStreamWriter& xml, const QString& tagName, const QDateTime& time) {
    xml.writeTextElement(tagName, time.toTimeSpec(Qt::UTC).toString(Qt::ISODate)); // YYYY-MM-DDTHH:mm:ssZ
}

/** Reads a Base-64 encoded UUID from current XML element; in case of error returns in case of error returns default (all-zero) UUID. */
PwUuid PwStreamUtilsV4::readUuid(QXmlStreamReader& xml) {
    QString base64Str = xml.readElementText();
    PwUuid result = PwUuid::fromBase64(base64Str);
    Util::safeClear(base64Str);
    return result;
}

/** Writes <tagName>UUID_as_base64</tagName> to the XML stream. */
void PwStreamUtilsV4::writeUuid(QXmlStreamWriter& xml, const QString& tagName, const PwUuid& uuid) {
    xml.writeTextElement(tagName, uuid.toByteArray().toBase64());
}

/** Reads a Base-64 encoded data array from current XML element; in case of error returns an empty array. */
QByteArray PwStreamUtilsV4::readBase64(QXmlStreamReader& xml) {
    QByteArray base64Str = xml.readElementText().toLatin1();
    QByteArray result = QByteArray::fromBase64(base64Str);
    Util::safeClear(base64Str);
    return result;
}

/** Writes <tagName>Base64_encoded_value</tagName> to the XML stream. */
void PwStreamUtilsV4::writeBase64(QXmlStreamWriter& xml, const QString& tagName, const QByteArray& value) {
    xml.writeTextElement(tagName, value.toBase64());
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
