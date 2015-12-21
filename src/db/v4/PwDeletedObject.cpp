/*
 * PwDeletedObject.cpp
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#include <PwDeletedObject.h>
#include "db/v4/PwStreamUtilsV4.h"

PwDeletedObject::PwDeletedObject(QObject* parent) :
        QObject(parent), _uuid(), _deletionTime(QDateTime::currentDateTime()) {
    // left empty
}

PwDeletedObject::PwDeletedObject(const PwUuid& uuid, QObject* parent) :
        QObject(parent), _uuid(uuid), _deletionTime(QDateTime::currentDateTime()) {
    // left empty
}

PwDeletedObject::~PwDeletedObject() {
    clear();
}

void PwDeletedObject::clear() {
    _uuid.clear();
    _deletionTime = QDateTime::currentDateTime(); // = now
}

ErrorCodesV4::ErrorCode PwDeletedObject::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_DELETED_OBJECT_ITEM);

    // All the fields will be read from the stream
    clear();

    bool conversionOk = true;
    ErrorCodesV4::ErrorCode err;

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_DELETED_OBJECT_ITEM == tagName))) {
        if (xml.isStartElement()) {
            if (XML_UUID == tagName) {
                _uuid = PwStreamUtilsV4::readUuid(xml);
            } else if (XML_DELETION_TIME == tagName) {
                _deletionTime = PwStreamUtilsV4::readTime(xml, &conversionOk);
                if (!conversionOk)
                    return ErrorCodesV4::XML_DELETED_OBJECT_ITEM_PARSING_ERROR_1;
            } else {
                LOG("unknown PwDeletedObject tag: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_DELETED_OBJECT_ITEM_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError())
        return ErrorCodesV4::XML_DELETED_OBJECT_ITEM_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;
}

void PwDeletedObject::writeToStream(QXmlStreamWriter& xml) {
    xml.writeStartElement(XML_DELETED_OBJECT_ITEM);
    PwStreamUtilsV4::writeUuid(xml, XML_UUID, getUuid());
    PwStreamUtilsV4::writeTime(xml, XML_DELETION_TIME, getDeletionTime());
    xml.writeEndElement();
}
