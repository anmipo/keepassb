/*
 * PwUuid.h
 *
 * Represents UUID of KeePass database entries/groups.
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
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

    /** Resets the UUID to all-zero bytes */
    void clear();

    QByteArray toByteArray() const { return Util::deepCopy(bytes); }

    /** Generates and returns a new UUID instance. */
    static PwUuid create();

    /**
     * Returns a new instance of PwUuid with the ID specified by a Base64 string.
     *
     * If the original string is empty, returns an all-zero UUID (not empty!)
     * This fixes an error which existed before KeePassB v2.4.2
     * (new groups had lastTopVisibleEntryUuid empty, which conflicted with KeePassX)
     */
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
