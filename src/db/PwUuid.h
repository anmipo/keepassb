/*
 * PwUuid.h
 *
 * Represents UUID of KeePass database entries/groups.
 *
 *  Created on: 25 Jun 2014
 *      Author: Andrei Popleteev
 */

#ifndef PWUUID_H_
#define PWUUID_H_

#include "util/Util.h"

class PwUuid {
private:
    QByteArray bytes;
public:
    PwUuid();
    PwUuid(const QByteArray& bytes);
    virtual ~PwUuid();

    void clear();
    QByteArray toByteArray() const { return bytes; }

    /** Generates and returns a new UUID instance. */
    static PwUuid create();

    // returns a new instance of PwUuid with the ID specified by a Base64 string
    static PwUuid fromBase64(const QString& base64);

    /** Returns true the UUID has not been set. */
    bool isEmpty() const { return bytes.isEmpty(); }
    /** Returns true the UUID is set to all-zeroes. */
    bool isAllZero() const { return Util::isAllZero(bytes); }

    QString toString() const { return bytes.toHex(); }

    inline bool operator==(const PwUuid& another) const { return (this->bytes == another.bytes); }
    inline bool operator!=(const PwUuid& another) const { return !operator==(another); }
    inline bool operator<(const PwUuid& another) const { return (this->bytes < another.bytes); }
};

#endif /* PWUUID_H_ */
