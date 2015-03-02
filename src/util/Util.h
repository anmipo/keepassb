/*
 * Util.h
 *
 * Application-wide utility methods.
 *
 *  Created on: 7 Jul 2014
 *      Author: Andrei Popleteev
 */

#ifndef UTIL_H_
#define UTIL_H_

class Util {
public:
    /**
     * Class-specific error codes
     */
    enum ErrorCode {
        SUCCESS             = 0,
        GZIP_DATA_TOO_SHORT = 1,
        GZIP_INIT_FAIL      = 2,
        GZIP_INFLATE_ERROR  = 3
    };
    /**
     * Unpacks GZip data.
     * This method is derived from http://stackoverflow.com/questions/13679592/gzip-in-blackberry-10
     */
    static ErrorCode inflateGZipData(const QByteArray& gzipData, QByteArray& outData);

    /**
     * Given a GZipped buffer, returns the size of its unpacked content.
     * In case of error returns -1.
     */
    static int getInflatedGZipSize(const QByteArray& gzipData);

    /**
     * Checks if data contains only zeros.
     */
    static bool isAllZero(const QByteArray& data);

    /**
     * Fills the array with a random byte value, then clears.
     */
    static void safeClear(QByteArray& data);
    /**
     * Fills the string data with a random byte, then clears.
     */
    static void safeClear(QString& string);
};

#endif /* UTIL_H_ */
