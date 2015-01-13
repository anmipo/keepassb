/*
 * PwGroupV4.cpp
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#include <PwGroupV4.h>
#include "db/v4/DefsV4.h"
#include "db/v4/PwEntryV4.h"
#include "db/v4/PwDatabaseV4.h"
#include "db/v4/PwStreamUtilsV4.h"
#include "util/Util.h"

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

/**
 * Creates a subgroup in the group and returns a reference to it.
 */
PwGroup* PwGroupV4::createGroup() {
    PwGroupV4* newGroup = new PwGroupV4(this);
    newGroup->setUuid(PwUuid::create());

    // inherit the icon and recycled status
    newGroup->setIconId(this->getIconId());
    newGroup->setDeleted(this->isDeleted());

    // set times
    newGroup->setCreationTime(QDateTime::currentDateTime());
    newGroup->setLastAccessTime(QDateTime::currentDateTime());
    newGroup->setLastModificationTime(QDateTime::currentDateTime());
    newGroup->setExpiryTime(QDateTime::currentDateTime());
    newGroup->setExpires(false);

    // set parent group
    newGroup->setParentGroup(this);
    this->addSubGroup(newGroup);
    return newGroup;
}

/**
 * Moves the group's whole branch to Backup group.
 * Returns true if successful.
 */
bool PwGroupV4::moveToBackup() {
    PwGroup* parentGroup = this->getParentGroup();
    if (!parentGroup) {
        qDebug() << "PwGroupV4::moveToBackup fail - no parent group";
        return false;
    }

    PwGroup* backupGroup = getDatabase()->getBackupGroup(true);
    if (!backupGroup) {
        qDebug() << "PwGroupV4::moveToBackup fail - no backup group created";
        return false;
    }

    parentGroup->removeSubGroup(this);
    backupGroup->addSubGroup(this);
    setParent(backupGroup); // parent in Qt terms, responsible for memory release

    // flag the group and all its children deleted
    setDeleted(true);
    QList<PwGroup*> childGroups;
    QList<PwEntry*> childEntries;
    getAllChildren(childGroups, childEntries);
    for (int i = 0; i < childGroups.size(); i++) {
        childGroups.at(i)->setDeleted(true);
    }
    for (int i = 0; i < childEntries.size(); i++) {
        childEntries.at(i)->setDeleted(true);
    }
    childGroups.clear();
    childEntries.clear();
    qDebug() << "PwGroupV4::moveToBackup OK";
    return true;
}

/**
 * Loads group fields from the stream.
 * The caller is responsible for clearing any previous values.
 */
ErrorCodesV4::ErrorCode PwGroupV4::readFromStream(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20) {
    Q_ASSERT(xml.name() == XML_GROUP);

    ErrorCodesV4::ErrorCode err;
    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_GROUP == tagName))) {
        if (xml.isStartElement()) {
            if (XML_UUID == tagName) {
                PwUuid uuid = PwStreamUtilsV4::readUuid(xml);
                setUuid(uuid);
                if ((uuid == meta.getRecycleBinGroupUuid()) && meta.isRecycleBinEnabled()) {
                    setDeleted(true); // may also be set higher in call stack
                }
            } else if (XML_NAME == tagName) {
                setName(PwStreamUtilsV4::readString(xml));
            } else if (XML_NOTES == tagName) {
                setNotes(PwStreamUtilsV4::readString(xml));
            } else if (XML_ICON_ID == tagName) {
                setIconId(PwStreamUtilsV4::readInt32(xml, 0));
            } else if (XML_TIMES == tagName) {
                err = readTimes(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (XML_IS_EXPANDED == tagName) {
                setIsExpanded(PwStreamUtilsV4::readBool(xml, true));
            } else if (XML_DEFAULT_AUTO_TYPE_SEQUENCE == tagName) {
                setDefaultAutoTypeSequence(PwStreamUtilsV4::readString(xml));
            } else if (XML_ENABLE_AUTO_TYPE == tagName) {
                setEnableAutoType(PwStreamUtilsV4::readString(xml)); // actually a bool, possibly "null"
            } else if (XML_ENABLE_SEARCHING == tagName) {
                setEnableSearching(PwStreamUtilsV4::readString(xml)); // actually a bool, possibly "null"
            } else if (XML_LAST_TOP_VISIBLE_ENTRY == tagName) {
                setLastTopVisibleEntry(PwStreamUtilsV4::readUuid(xml));
            } else if (XML_GROUP == tagName) {
                PwGroupV4* subGroup = new PwGroupV4(this);
                subGroup->setDatabase(this->getDatabase());
                err = subGroup->readFromStream(xml, meta, salsa20);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;

                if (isDeleted())
                    subGroup->setDeleted(true); // propagate the deleted flag recursively
                this->addSubGroup(subGroup);
            } else if (XML_ENTRY == tagName) {
                PwEntryV4* entry = new PwEntryV4(this);
                err = entry->readFromStream(xml, meta, salsa20);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;

                entry->setDeleted(this->isDeleted()); // propagate the deleted flag recursively
                this->addEntry(entry);
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError())
        return ErrorCodesV4::XML_GROUP_PARSING_ERROR;
    else
        return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwGroupV4::readTimes(QXmlStreamReader& xml) {
    Q_ASSERT(XML_TIMES == xml.name());

    QString text;
    xml.readNext();
    QStringRef tagName = xml.name();
    bool conversionOk = true;
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_TIMES))) {
        if (xml.isStartElement()) {
            if (tagName == XML_LAST_MODIFICATION_TIME) {
                setLastModificationTime(PwStreamUtilsV4::readTime(xml, &conversionOk));
            } else if (tagName == XML_CREATION_TIME) {
                setCreationTime(PwStreamUtilsV4::readTime(xml, &conversionOk));
            } else if (tagName == XML_LAST_ACCESS_TIME) {
                setLastAccessTime(PwStreamUtilsV4::readTime(xml, &conversionOk));
            } else if (tagName == XML_EXPIRY_TIME) {
                setExpiryTime(PwStreamUtilsV4::readTime(xml, &conversionOk));
            } else if (tagName == XML_EXPIRES) {
                setExpires(PwStreamUtilsV4::readBool(xml, false));
            } else if (tagName == XML_USAGE_COUNT) {
                setUsageCount(PwStreamUtilsV4::readUInt32(xml, 0));
            } else if (tagName == XML_LOCATION_CHANGED_TIME) {
                setLocationChangedTime(PwStreamUtilsV4::readTime(xml, &conversionOk));
            } else {
                qDebug() << "unknown PwGroupV4/Times tag:" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
            }
        }
        if (!conversionOk)
            break;
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError() || !conversionOk)
        return ErrorCodesV4::XML_GROUP_TIMES_PARSING_ERROR;
    else
        return ErrorCodesV4::SUCCESS;
}

void PwGroupV4::clear() {
    _isExpanded = true;
    Util::safeClear(_defaultAutoTypeSequence);
    Util::safeClear(_enableAutoType);
    Util::safeClear(_enableSearching);
    _lastTopVisibleEntryUuid.clear();
    _usageCount = 0;

    QDateTime now = QDateTime::currentDateTime();
    _locationChangedTime = now;

    PwGroup::clear();
}

void PwGroupV4::setIsExpanded(bool expanded) {
    if (expanded != _isExpanded) {
        _isExpanded = expanded;
        emit expandedChanged(expanded);
    }
}

void PwGroupV4::setDefaultAutoTypeSequence(const QString& defaultAutoTypeSequence) {
    if (defaultAutoTypeSequence != _defaultAutoTypeSequence) {
        _defaultAutoTypeSequence = defaultAutoTypeSequence;
        emit defaultAutoTypeSequenceChanged(defaultAutoTypeSequence);
    }
}

void PwGroupV4::setEnableAutoType(const QString& enableAutoType) {
    if (enableAutoType != _enableAutoType) {
        _enableAutoType = enableAutoType;
        emit enableAutoTypeChanged(enableAutoType);
    }
}

void PwGroupV4::setEnableSearching(const QString& enableSearching) {
    if (enableSearching != _enableSearching) {
        _enableSearching = enableSearching;
        emit enableSearchingChanged(enableSearching);
    }
}

void PwGroupV4::setLastTopVisibleEntry(const PwUuid& uuid) {
    if (uuid != _lastTopVisibleEntryUuid) {
        _lastTopVisibleEntryUuid = uuid;
        emit lastTopVisibleEntryChanged(uuid);
    }
}

void PwGroupV4::setUsageCount(const quint32 usageCount) {
    if (usageCount != _usageCount) {
        _usageCount = usageCount;
        emit usageCountChanged(usageCount);
    }
}

void PwGroupV4::setLocationChangedTime(const QDateTime& locationChangedTime) {
    if (locationChangedTime != _locationChangedTime) {
        _locationChangedTime = locationChangedTime;
        emit locationChangedTimeChanged(locationChangedTime);
    }
}
