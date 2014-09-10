/*
 * PwGroupV3.h
 *
 *  Created on: 19 Aug 2014
 *      Author: Andrei
 */

#ifndef PWGROUPV3_H_
#define PWGROUPV3_H_

#include <QObject>
#include "db/PwGroup.h"

class PwGroupV3: public PwGroup {
    Q_OBJECT
private:
    qint32 _id;
    quint16 _level;
    qint32 _flags;   // some internal KeePass field

    const static quint16 FIELD_RESERVED  = 0x0000;
    const static quint16 FIELD_GROUP_ID  = 0x0001;
    const static quint16 FIELD_NAME      = 0x0002;
    const static quint16 FIELD_CREATION_TIME      = 0x0003;
    const static quint16 FIELD_LAST_MODIFIED_TIME = 0x0004;
    const static quint16 FIELD_LAST_ACCESS_TIME   = 0x0005;
    const static quint16 FIELD_EXPIRATION_TIME    = 0x0006;
    const static quint16 FIELD_ICON_ID            = 0x0007;
    const static quint16 FIELD_GROUP_LEVEL        = 0x0008;
    const static quint16 FIELD_GROUP_FLAGS        = 0x0009;
    const static quint16 FIELD_END                = 0xFFFF;

public:
    PwGroupV3(QObject* parent = 0);
    virtual ~PwGroupV3();

    /** Loads group fields from the stream. Returns true on success, false in case of error. */
    bool readFromStream(QDataStream& stream);

    virtual void clear();

    // property accessors
    void setId(const qint32 id) { this->_id = id; }
    qint32 getId() const { return _id; }
    void setLevel(const quint16 level) { this->_level = level; }
    quint16 getLevel() const { return _level; }
    void setFlags(const qint32 flags) { this->_flags = flags; }
    qint32 getFlags() const { return _flags; }
};

#endif /* PWGROUPV3_H_ */
