/*
 * ProgressObserver.cpp
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#include "util/ProgressObserver.h"

/**
 * Sets starting and finishing progress percentage for the current processing phase.
 * (For example, saving XML might be mapped to the 70..100% range of a progress bar)
 */
void ProgressObserver::setPhaseProgressBounds(const quint8* boundsInPercent) {
    fromPercent = boundsInPercent[0];
    toPercent = boundsInPercent[1];
    setPhaseProgressRawTarget(1);
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
    resetProgress();
}

/**
 * Updates raw progress counter.
 * Safe for frequent calls: onProgress() is called only when progress percentage has changed.
 */
void ProgressObserver::setProgress(quint32 rawProgress) {
    this->rawProgress = rawProgress;
    int newPercent = getProgressPercent(rawProgress);
    if (progressPercent != newPercent) {
        progressPercent = newPercent;
        onProgress(progressPercent);
    }
}

/**
 * Increases raw progress by the given value.
 * Safe for frequent calls: onProgress() is called only when progress percentage has changed.
 */
void ProgressObserver::increaseProgress(quint32 rawDelta) {
    setProgress(rawProgress + rawDelta);
}

/** Resets raw progress counter. */
void ProgressObserver::resetProgress() {
    setProgress(0);
}
