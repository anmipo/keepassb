/*
 * PwGroupV4.h
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#ifndef PWGROUPV4_H_
#define PWGROUPV4_H_

#include "db/PwGroup.h"

class PwGroupV4: public PwGroup {
    Q_OBJECT
public:
    PwGroupV4();
    virtual ~PwGroupV4();
};

Q_DECLARE_METATYPE(PwGroupV4*);

#endif /* PWGROUPV4_H_ */
