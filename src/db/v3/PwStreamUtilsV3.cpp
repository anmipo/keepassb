/*
 * PwStreamUtilsV3.cpp
 *
 *  Created on: 11 Sep 2014
 *      Author: Andrei
 */

#include <PwStreamUtilsV3.h>
#include "util/Util.h"

/** Reads a 5-byte V3-specific timestamp from the stream */
QDateTime PwStreamUtilsV3::readTimestamp(QDataStream& stream) {
    quint8 dw1, dw2, dw3, dw4, dw5;
    stream >> dw1 >> dw2 >> dw3 >> dw4 >> dw5; // 31, 122, 33, 42, 210

    int year = (dw1 << 6) | (dw2 >> 2); // 2014
    int month = ((dw2 & 0x00000003) << 2) | (dw3 >> 6); // 8
    int day = (dw3 >> 1) & 0x0000001F; // 16
    QDate date(year, month, day);

    int hour = ((dw3 & 0x00000001) << 4) | (dw4 >> 4); // 18
    int minute = ((dw4 & 0x0000000F) << 2) | (dw5 >> 6); // 43
    int second = dw5 & 0x0000003F; // 18
    QTime time(hour, minute, second, 0);

    QDateTime result(date, time, Qt::UTC);
    return result;
}

/** Writes the timestamp to the stream in a packed 5-byte format */
void PwStreamUtilsV3::writeTimestamp(QDataStream& stream, const QDateTime& timestamp) {
    QDate date = timestamp.date();
    QTime time = timestamp.time();
    char pack[5];
    pack[0] = (date.year() >> 6) & 0x0000003F;
    pack[1] = ((date.year()   & 0x0000003F) << 2) | ((date.month() >> 2) & 0x00000003);
    pack[2] = ((date.month()  & 0x00000003) << 6) | ((date.day() & 0x0000001F) << 1) | ((time.hour() >> 4) & 0x00000001);
    pack[3] = ((time.hour()   & 0x0000000F) << 4) | ((time.minute() >> 2) & 0x0000000F);
    pack[4] = ((time.minute() & 0x00000003) << 6) | (time.second() & 0x0000003F);
    stream.writeBytes(pack, 5);
}

QString PwStreamUtilsV3::readString(QDataStream& stream, qint32 fieldSize) {
    QByteArray buf(fieldSize, 0);
    stream.readRawData(buf.data(), fieldSize);
    QString result = QString::fromUtf8(buf.constData());
    Util::safeClear(buf);
    return result;
}

void PwStreamUtilsV3::writeString(QDataStream& stream, QString s) {
    QByteArray buf = s.toUtf8();
    qint32 fieldSize = buf.size() + 1;
    stream << fieldSize;
    stream.writeRawData(buf.constData(), fieldSize);
}

QByteArray PwStreamUtilsV3::readData(QDataStream& stream, qint32 fieldSize) {
    QByteArray res(fieldSize, 0);
    stream.readRawData(res.data(), fieldSize);
    return res;
}

qint32 PwStreamUtilsV3::readInt32(QDataStream& stream) {
    qint32 result;
    stream >> result;
    return result;
}

quint16 PwStreamUtilsV3::readUInt16(QDataStream& stream) {
    quint16 result;
    stream >> result;
    return result;
}

void PwStreamUtilsV3::writeData(QDataStream& stream, const QByteArray& value) {
    stream << (qint32)value.size();
    stream.writeRawData(value.constData(), value.size());
}

void PwStreamUtilsV3::writeInt32(QDataStream& stream, qint32 value) {
    stream << (qint32)sizeof(value) << value;
}

void PwStreamUtilsV3::writeUInt16(QDataStream& stream, quint16 value) {
    stream << (qint32)sizeof(value) << value;
}
