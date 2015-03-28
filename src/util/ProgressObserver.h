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
public:
    /**
     * Progress update of XML parsing.
     * xmlPos is characterOffset value of the XML stream
     */
    virtual void onXmlProgress(qint64 xmlPos) = 0;
};


#endif /* PROGRESSOBSERVER_H_ */
