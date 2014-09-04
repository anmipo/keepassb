/*
 * PwGroupV4.cpp
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#include <PwGroupV4.h>

PwGroupV4::PwGroupV4(QObject* parent) : PwGroup(parent) {
    // nothing to do here
}

PwGroupV4::~PwGroupV4() {
    clear();
}

