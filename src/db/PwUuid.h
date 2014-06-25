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

class PwUuid {
private:
    QByteArray bytes;
public:
    PwUuid();
    PwUuid(const QByteArray& bytes);
    virtual ~PwUuid();

    void clear();

    // returns a new instance of PwUuid with the ID specified by a Base64 string
    static PwUuid fromBase64(const QString& base64);

    virtual bool equals(const PwUuid& another) const;

    inline bool operator==(const PwUuid& another) { return (this->bytes == another.bytes); }
};

#endif /* PWUUID_H_ */
