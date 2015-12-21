/*
 * PwDeletedObject.h
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#ifndef PWDELETEDOBJECT_H_
#define PWDELETEDOBJECT_H_

#include "db/v4/DefsV4.h"
#include <QtXml/QXmlStreamReader>

/**
 * V4 databases may maintain a list of deleted objects.
 * This class represents items of this list.
 */
class PwDeletedObject: public QObject {
    Q_OBJECT
private:
    PwUuid _uuid;
    QDateTime _deletionTime;
public:
    PwDeletedObject(QObject* parent=0);
    PwDeletedObject(const PwUuid& uuid, QObject* parent=0);
    virtual ~PwDeletedObject();

    void clear();

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml);
    void writeToStream(QXmlStreamWriter& xml);

    //property accessors
    PwUuid getUuid() const { return _uuid; }
    QDateTime getDeletionTime() const { return _deletionTime; }
};

#endif /* PWDELETEDOBJECT_H_ */
