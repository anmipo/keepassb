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
public:
    PwGroupV3(QObject* parent = 0);
    virtual ~PwGroupV3();

    // property accessors
    void setId(const qint32 id) { this->_id = id; }
    qint32 getId() const { return _id; }
    void setLevel(const quint16 level) { this->_level = level; }
    quint16 getLevel() const { return _level; }
    void setFlags(const qint32 flags) { this->_flags = flags; }
    qint32 getFlags() const { return _flags; }
};

#endif /* PWGROUPV3_H_ */
