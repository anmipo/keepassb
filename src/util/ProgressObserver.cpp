/*
 * ProgressObserver.cpp
 *
 *  Created on: 28 Mar 2015
 *      Author: Andrei Popleteev
 */

#include "util/ProgressObserver.h"

/**
 * Sets starting and finishing progress percentage for the current processing phase.
 * (For example, saving XML might be mapped to the 70..100% range of a progress bar)
 */
void ProgressObserver::setPhaseProgressBounds(const quint8* boundsInPercent) {
    fromPercent = boundsInPercent[0];
    toPercent = boundsInPercent[1];
}

/**
 * Converts current raw progress to absolute progress percentage,
 * considering phase progress bounds.
 */
int ProgressObserver::getProgressPercent(quint32 rawProgress) {
    return (int)(fromPercent + ((float)rawProgress / rawMax) * (toPercent - fromPercent));
}

/**
 * Sets maximum raw progress value in this phase.
 * (In onProgress, rawProgress parameter can range from 0 to rawMax)
 */
void ProgressObserver::setPhaseProgressRawTarget(quint32 rawMax) {
    this->rawMax = rawMax;
}
