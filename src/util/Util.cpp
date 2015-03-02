/*
 * Util.cpp
 *
 * Application-wide utility methods.
 *
 *  Created on: 7 Jul 2014
 *      Author: Andrei Popleteev
 */

#include "Util.h"

#include <QDebug>
#include <zlib.h>

/**
 * Unpacks GZip data.
 * This method is derived from http://stackoverflow.com/questions/13679592/gzip-in-blackberry-10
 */
Util::ErrorCode Util::inflateGZipData(const QByteArray& gzipData, QByteArray& outData) {
    if (gzipData.size() <= 4) {
        qDebug() << "inflateGZipData: Input data is too short";
        return GZIP_DATA_TOO_SHORT;
    }

    int err;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = gzipData.size();
    strm.next_in = (Bytef*)(gzipData.data());

    err = inflateInit2(&strm, 16 + MAX_WBITS); // gzip decoding
    if (err != Z_OK) {
        qDebug() << "inflateGZipData: inflateInit2 error " << err;
        return GZIP_INIT_FAIL;
    }

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        err = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(err != Z_STREAM_ERROR);  // state not clobbered

        switch (err) {
        case Z_NEED_DICT:
            err = Z_DATA_ERROR;     // and fall through
            /* no break */
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            qDebug() << "inflateGZipData: inflate error " << err;
            return GZIP_INFLATE_ERROR;
        }
        outData.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);
    // clean up and return
    inflateEnd(&strm);
    return SUCCESS;
}

/**
 * Given a GZipped buffer, returns the size of its unpacked content.
 * In case of error returns -1.
 */
int Util::getInflatedGZipSize(const QByteArray& gzipData) {
    // TODO optimise this by getting rid of memory allocation/cleaning.
    QByteArray inflatedData;
    int result = -1;
    if (Util::inflateGZipData(gzipData, inflatedData) == SUCCESS) {
        result = inflatedData.size();
    }
    safeClear(inflatedData);
    return result;
}

/**
 * Checks if data contains only zeros.
 */
bool Util::isAllZero(const QByteArray& data) {
    for (int i = 0; i < data.length(); i++)
        if (data[i] != '\x00')
            return false;
    return true;
}

/**
 * Fills the array with a random byte value, then clears.
 */
void Util::safeClear(QByteArray& data) {
    char randomByte = qrand() % 0xFF;
    data.fill(randomByte);
    data.clear();
}

/**
 * Fills the string data with a random byte, then clears.
 */
void Util::safeClear(QString& string) {
    QChar randomChar(qrand() % 0xFF);
    string.fill(randomChar);
    string.clear();
}
