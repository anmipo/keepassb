/*
 * PwStreamUtilsV4.h
 *
 *  Created on: 1 Dec 2014
 *      Author: Andrei
 */

#ifndef PWSTREAMUTILSV4_H_
#define PWSTREAMUTILSV4_H_

#include <QtXml/QXmlStreamReader>
#include <QDateTime.h>
#include "db/PwUuid.h"

struct PwStreamUtilsV4
{
    /** Reads a boolean from current XML element; in case of errors returns the default value. */
    static bool readBool(QXmlStreamReader& xml, bool defaultValue);
    /** Writes <tagName>bool_value_as_string</tagName> to the XML stream. */
    static void writeBool(QXmlStreamWriter& xml, const QString& tagName, const bool value);

    /** Reads an uint from current XML element; in case of errors returns the default value. */
    static quint32 readUInt32(QXmlStreamReader& xml, quint32 defaultValue);
    /** Writes <tagName>value</tagName> to the XML stream. */
    static void writeUInt32(QXmlStreamWriter& xml, const QString& tagName, const quint32 value);

    /** Reads a signed int from current XML element; in case of errors returns the default value. */
    static qint32 readInt32(QXmlStreamReader& xml, qint32 defaultValue);
    /** Writes <tagName>value</tagName> to the XML stream. */
    static void writeInt32(QXmlStreamWriter& xml, const QString& tagName, const qint32 value);

    /** Reads a long from current XML element; in case of errors returns the default value. */
    static qint64 readInt64(QXmlStreamReader& xml, qint64 defaultValue);
    /** Writes <tagName>value</tagName> to the XML stream. */
    static void writeInt64(QXmlStreamWriter& xml, const QString& tagName, const qint64 value);

    /** Reads a string from current XML element. */
    static QString readString(QXmlStreamReader& xml);
    /** Writes <tagName>text</tagName> to the XML stream. If text is empty, writes an empty <tagName/>. */
    static void writeString(QXmlStreamWriter& xml, const QString& tagName, const QString& text);

    /**
     * Reads a UTC timestamp from current XML element; in case of error returns current time.
     * If okPtr is specified, it will set to indicate the success of the conversion.
     */
    static QDateTime readTime(QXmlStreamReader& xml, bool *okPtr=NULL);
    /** Writes <tagName>time_value</tagName> to the XML stream. */
    static void writeTime(QXmlStreamWriter& xml, const QString& tagName, const QDateTime& time);

    /** Reads a Base-64 encoded UUID from current XML element; in case of error returns default (all-zero) UUID. */
    static PwUuid readUuid(QXmlStreamReader& xml);
    /** Writes <tagName>UUID_as_base64</tagName> to the XML stream. */
    static void writeUuid(QXmlStreamWriter& xml, const QString& tagName, const PwUuid& uuid);

    /** Reads a Base-64 encoded data array from current XML element; in case of error returns an empty array. */
    static QByteArray readBase64(QXmlStreamReader& xml);
    /** Writes <tagName>Base64_encoded_value</tagName> to the XML stream. */
    static void writeBase64(QXmlStreamWriter& xml, const QString& tagName, const QByteArray& value);


    /** Skips to the end tag of an unknown element, ignoring any nested elements. */
    static void readUnknown(QXmlStreamReader& xml);
};

#endif /* PWSTREAMUTILSV4_H_ */
