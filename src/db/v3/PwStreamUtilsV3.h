/*
 * PwStreamUtilsV3.h
 *
 *  Created on: 11 Sep 2014
 *      Author: Andrei Popleteev
 */

#ifndef PWSTREAMUTILSV3_H_
#define PWSTREAMUTILSV3_H_

#include <QDataStream.h>
#include <QDateTime.h>

struct PwStreamUtilsV3
{
    /** Reads a 5-byte V3-specific timestamp from the stream */
    static QDateTime readTimestamp(QDataStream& stream);

    /** Writes a 5-byte V3-specific timestamp from the stream */
    static void writeTimestamp(QDataStream& stream, const QDateTime& timestamp);

    /** Reads a null-terminated string (of fieldSize bytes) from the stream. */
    static QString readString(QDataStream& stream, qint32 fieldSize);

    /** Writes the size of the string and its UTF-8 bytes to the stream, ensuring there is a null at the end */
    static void writeString(QDataStream& stream, QString s);

    static QByteArray readData(QDataStream& stream, qint32 fieldSize);
    static void writeData(QDataStream& stream, const QByteArray& value);

    static qint32 readInt32(QDataStream& stream);
    static void writeInt32(QDataStream& stream, qint32 value);

    static quint16 readUInt16(QDataStream& stream);
    static void writeUInt16(QDataStream& stream, quint16 value);
};

#endif /* PWSTREAMUTILSV3_H_ */
