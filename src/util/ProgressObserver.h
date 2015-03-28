/*
 * ProgressObserver.h
 *
 *  Created on: 28 Mar 2015
 *      Author: Andrei Popleteev
 */

#ifndef PROGRESSOBSERVER_H_
#define PROGRESSOBSERVER_H_

/**
 * Interface for classes monitoring the progress of time-consuming processes.
 */
class ProgressObserver {
private:
    int fromPercent;
    int toPercent;
    int rawMax;
    int rawProgress;
    int progressPercent;
protected:
    /**
     * Sets starting and finishing progress percentage for the current processing phase.
     * (For example, saving XML might be mapped to the 70..100% range of a progress bar)
     */
    void setPhaseProgressBounds(const quint8* boundsInPercent);
    /**
     * Converts current raw progress to absolute progress percentage,
     * considering phase progress bounds.
     */
    int getProgressPercent(quint32 rawProgress);

    /**
     * Progress update.
     */
    virtual void onProgress(quint8 progressPercent) = 0;
public:
    /**
     * Sets maximum raw progress value in this phase.
     * (In onProgress, rawProgress parameter can range from 0 to rawMax)
     */
    void setPhaseProgressRawTarget(quint32 rawMax);

    /**
     * Updates raw progress counter and calls onProgress() if progress percentage changed.
     * Safe for frequent calls: onProgress() is called only when progress percentage has changed.
     */
    void setProgress(quint32 rawProgress);
    /**
     * Increases raw progress by the given value.
     * Safe for frequent calls: onProgress() is called only when progress percentage has changed.
     */
    void increaseProgress(quint32 rawDelta);
    /** Resets raw progress counter. */
    void resetProgress();
};


#endif /* PROGRESSOBSERVER_H_ */
