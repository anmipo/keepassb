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

    /** Reads a long from current XML element; in case of errors returns the default value. */
    static qint64 readInt64(QXmlStreamReader& xml, qint64 defaultValue);

    /** Reads an uint from current XML element; in case of errors returns the default value. */
    static quint32 readUInt32(QXmlStreamReader& xml, quint32 defaultValue);

    /** Reads a signed int from current XML element; in case of errors returns the default value. */
    static qint32 readInt32(QXmlStreamReader& xml, qint32 defaultValue);

    /** Reads a string from current XML element. */
    static QString readString(QXmlStreamReader& xml);

    /**
     * Reads a UTC timestamp from current XML element; in case of error returns current time.
     * If okPtr is specified, it will set to indicate the success of the conversion.
     */
    static QDateTime readTime(QXmlStreamReader& xml, bool *okPtr=NULL);

    /** Reads a Base-64 encoded UUID from current XML element; in case of error returns default (all-zero) UUID. */
    static PwUuid readUuid(QXmlStreamReader& xml);

    /** Reads a Base-64 encoded data array from current XML element; in case of error returns an empty array. */
    static QByteArray readBase64(QXmlStreamReader& xml);

    /** Skips to the end tag of an unknown element, ignoring any nested elements. */
    static void readUnknown(QXmlStreamReader& xml);
};

#endif /* PWSTREAMUTILSV4_H_ */
