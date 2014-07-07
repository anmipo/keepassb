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
        SUCCESS = 0,
        GZIP_DATA_TOO_SHORT,
        GZIP_INIT_FAIL,
        GZIP_INFLATE_ERROR
    };
    /**
     * Unpacks GZip data.
     * This method is derived from http://stackoverflow.com/questions/13679592/gzip-in-blackberry-10
     */
    static ErrorCode inflateGZipData(const QByteArray& gzipData, QByteArray& outData);
};

#endif /* UTIL_H_ */
