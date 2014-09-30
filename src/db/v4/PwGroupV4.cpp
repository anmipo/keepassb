/*
 * PwGroupV4.cpp
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#include <PwGroupV4.h>
#include "db/v4/PwEntryV4.h"

PwGroupV4::PwGroupV4(QObject* parent) : PwGroup(parent) {
    // nothing to do here
}

PwGroupV4::~PwGroupV4() {
    clear();
}

/**
 * Creates an entry in the group and returns a reference to it.
 */
PwEntry* PwGroupV4::createEntry() {
    PwEntryV4* newEntry = new PwEntryV4(this);
    newEntry->setUuid(PwUuid::create());

    // inherit the icon and recycled status
    newEntry->setIconId(this->getIconId());
    newEntry->setDeleted(this->isDeleted());

    // set times
    newEntry->setCreationTime(QDateTime::currentDateTime());
    newEntry->setLastAccessTime(QDateTime::currentDateTime());
    newEntry->setLastModificationTime(QDateTime::currentDateTime());

    newEntry->setExpiryTime(QDateTime::currentDateTime());
    newEntry->setExpires(false);

    // set parent group
    newEntry->setParentGroup(this);
    this->addEntry(newEntry);
    return newEntry;
}
