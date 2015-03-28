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
#include <zconf.h>

// Gzip ideally needs 128-256 K buffers; but default QNX stack size on ARM is 128 K
// (www.qnx.com/support/knowledgebase.html?id=50130000000PRnk)
// Therefore allocate gzip buffers in heap.
static const int GZIP_CHUNK_SIZE = 128 * 1024;

/**
 * Unpacks GZip data.
 * This method is derived from http://stackoverflow.com/questions/13679592/gzip-in-blackberry-10
 */
Util::ErrorCode Util::inflateGZipData(const QByteArray& gzipData, QByteArray& outData, ProgressObserver* progressObserver) {
    if (gzipData.size() <= 4) {
        qDebug() << "inflateGZipData: Input data is too short";
        return GZIP_INFLATE_DATA_TOO_SHORT;
    }

    if (progressObserver) {
        //TODO make finer-grained progress reporting
        progressObserver->setPhaseProgressRawTarget(1);
        progressObserver->onProgress(0);
    }

    outData.clear();

    z_stream strm;
    QByteArray outBuf(GZIP_CHUNK_SIZE, 0);
    char* out = outBuf.data();

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = gzipData.size();
    strm.next_in = (Bytef*)(gzipData.data());

    int err = inflateInit2(&strm, 16 + MAX_WBITS); // gzip decoding
    if (err != Z_OK) {
        qDebug() << "inflateGZipData: inflateInit2 error " << err;
        return GZIP_INFLATE_INIT_FAIL;
    }

    // run inflate()
    do {
        strm.avail_out = GZIP_CHUNK_SIZE;
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
        outData.append(out, GZIP_CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    safeClear(outBuf);
    inflateEnd(&strm);

    if (progressObserver)
        progressObserver->onProgress(1);

    return SUCCESS;
}

/**
 * Compresses data to GZip format.
 */
Util::ErrorCode Util::compressToGZip(const QByteArray& inData, QByteArray& gzipData, ProgressObserver* progressObserver) {
    gzipData.clear();

    QByteArray outBuf(GZIP_CHUNK_SIZE, 0);
    char* out = outBuf.data();

    if (progressObserver) {
        //TODO make finer-grained progress reporting
        progressObserver->setPhaseProgressRawTarget(1);
        progressObserver->onProgress(0);
    }

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = inData.size();
    strm.next_in = (Bytef*)(inData.data());

    int err = deflateInit2(&strm,
            Z_BEST_COMPRESSION,
            Z_DEFLATED,
            16 + MAX_WBITS,   // request gzip (instead of zlib) stream
            MAX_MEM_LEVEL,    // trade some memory for faster processing
            Z_DEFAULT_STRATEGY);
    if (err != Z_OK) {
        qDebug() << "compressToGzip: deflateInit2 error " << err;
        return GZIP_DEFLATE_INIT_FAIL;
    }

    do {
        strm.avail_out = GZIP_CHUNK_SIZE;
        strm.next_out = (Bytef*)out;

        // Flush flag is Z_FINISH because we already have the complete input buffer, no additions expected.
        err = deflate(&strm, Z_FINISH);
        // ensure the stream state is not damaged from elsewhere (a rather redundant check)
        Q_ASSERT(err != Z_STREAM_ERROR);
        // No need to check return code, since "deflate() can do no wrong here" (zlib.net/zlib_how.html)

        gzipData.append(out, GZIP_CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    safeClear(outBuf);
    deflateEnd(&strm);

    if (progressObserver)
        progressObserver->onProgress(1);

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
 * Fills the array with a random byte value.
 */
void Util::randomFill(char* data, int size) {
    int randomValue = qrand();
    // N.B.: memset() might be optimized out, so force clearing with memset_s()
    // (see www.qnx.com/developers/docs/660/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmemset_s.html)
    memset_s(data, size, randomValue, size);
}

/**
 * Fills the array with a random byte value, then clears.
 */
void Util::safeClear(QByteArray& data) {
    randomFill(data.data(), data.size());
    data.clear();
}

/**
 * Fills the string data with a random byte, then clears.
 */
void Util::safeClear(QString& string) {
    randomFill((char*)string.data(), string.size() * sizeof(QChar));
    string.clear();
}

/**
 * Makes a deep copy of the object.
 */
QString Util::deepCopy(const QString& string) {
    QString copy = QString(string.constData(), string.size()); // makes a deep copy
    return copy; // returns an implicitly-shared copy of the deep copy
}

/**
 * Makes a deep copy of the object.
 */
QByteArray Util::deepCopy(const QByteArray& data) {
    QByteArray copy = QByteArray(data.constData(), data.size()); // makes a deep copy
    return copy; // returns an implicitly-shared copy of the deep copy
}
