/*
 * PwMetaV4.cpp
 *
 *  Created on: 11 Dec 2014
 *      Author: Andrei Popleteev
 */


#include "db/v4/PwMetaV4.h"
#include "db/PwUuid.h"
#include "util/Util.h"
#include "db/v4/PwStreamUtilsV4.h"


// Some database defaults
int DEFAULT_HISTORY_MAX_ITEMS = 10; // -1 for unlimited
int DEFAULT_HISTORY_MAX_SIZE = 6 * 1024 * 1024; // -1 for unlimited
int DEFAULT_MAINTENANCE_HISTORY_DAYS = 365;


MemoryProtection::MemoryProtection(QObject* parent) : QObject(parent) {
    clear(); // reset to default values
}

MemoryProtection::~MemoryProtection() {
    // empty
}

void MemoryProtection::clear() {
    protectTitle = false;
    protectUserName = false;
    protectPassword = true;
    protectUrl = false;
    protectNotes = false;
}

QString MemoryProtection::toString() const {
    return QString("{title: %1, username: %2, password: %3, url: %4, notes: %5}")
            .arg(protectTitle)
            .arg(protectUserName)
            .arg(protectPassword)
            .arg(protectUrl)
            .arg(protectNotes);
}

ErrorCodesV4::ErrorCode MemoryProtection::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_MEMORY_PROTECTION);

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_MEMORY_PROTECTION == tagName))) {
        if (xml.isStartElement()) {
            if (XML_MEMORY_PROTECTION_PROTECT_TITLE == tagName) {
                protectTitle = PwStreamUtilsV4::readBool(xml, false);
            } else if (XML_MEMORY_PROTECTION_PROTECT_USERNAME == tagName) {
                protectUserName = PwStreamUtilsV4::readBool(xml, false);
            } else if (XML_MEMORY_PROTECTION_PROTECT_PASSWORD == tagName) {
                protectPassword = PwStreamUtilsV4::readBool(xml, true);
            } else if (XML_MEMORY_PROTECTION_PROTECT_URL == tagName) {
                protectUrl = PwStreamUtilsV4::readBool(xml, false);
            } else if (XML_MEMORY_PROTECTION_PROTECT_NOTES == tagName) {
                protectNotes = PwStreamUtilsV4::readBool(xml, false);
            } else {
                qDebug() << "WARN: unknown MemoryProtection tag " << tagName;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_MEMORY_PROTECTION_PARSING_ERROR;
    else
        return ErrorCodesV4::SUCCESS;
}

void MemoryProtection::writeToStream(QXmlStreamWriter& xml) {
    xml.writeStartElement(XML_MEMORY_PROTECTION);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_TITLE, protectTitle);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_USERNAME, protectUserName);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_PASSWORD, protectPassword);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_URL, protectUrl);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_NOTES, protectNotes);
    xml.writeEndElement(); // XML_MEMORY_PROTECTION
}

/****************************/
PwBinaryV4::PwBinaryV4(QObject* parent) : QObject(parent) {
    // nothing to do
}
PwBinaryV4::~PwBinaryV4() {
    clear();
}
void PwBinaryV4::clear() {
    _isCompressed = false;
    Util::safeClear(_data);
}

bool PwBinaryV4::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_BINARY);

    if (xml.isStartElement() && (xml.name() == XML_BINARY)) {
        QXmlStreamAttributes attrs = xml.attributes();
        _id = attrs.value(XML_BINARY_ID).toString();
        _isCompressed = (attrs.value(XML_BINARY_COMPRESSED) == XML_TRUE);
        _data = PwStreamUtilsV4::readBase64(xml);
        // data might probably be empty
    } else {
        qDebug() << "invalid Binary structure, got" << xml.name() << "tag";
        return false;
    }
    return (!xml.hasError() && !_id.isEmpty());
}

void PwBinaryV4::writeToStream(QXmlStreamWriter& xml) {
    xml.writeStartElement(XML_BINARY);
    xml.writeAttribute(XML_BINARY_ID, _id);
    xml.writeAttribute(XML_BINARY_COMPRESSED, _isCompressed ? XML_TRUE : XML_FALSE);
    xml.writeCharacters(_data.toBase64());
    xml.writeEndElement(); // XML_BINARY
}

QString PwBinaryV4::toString() const {
    return QString("{ID: %1, compressed: %2, data.size: %3}")
            .arg(_id).arg(_isCompressed).arg(_data.size());
}
/****************************/
PwCustomIconV4::PwCustomIconV4(QObject* parent) : QObject(parent) {
    // nothing to do here
}

PwCustomIconV4::~PwCustomIconV4() {
    clear();
}

void PwCustomIconV4::clear() {
    uuid.clear();
    Util::safeClear(data);
}

PwUuid PwCustomIconV4::getUuid() const {
    return uuid;
}

QByteArray PwCustomIconV4::getData() const {
    return data;
}

bool PwCustomIconV4::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_CUSTOM_ICON_ITEM);

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_CUSTOM_ICON_ITEM == tagName))) {
        if (xml.isStartElement()) {
            if (tagName == XML_CUSTOM_ICON_ITEM_ID) {
                uuid = PwStreamUtilsV4::readUuid(xml);
            } else if (tagName == XML_CUSTOM_ICON_ITEM_DATA) {
                data = PwStreamUtilsV4::readBase64(xml);
            } else {
                qDebug() << "unexpected XML tag in CustomIcon: " << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return false;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    return (!xml.hasError() && !data.isEmpty());
}

/** Writes icon fields to an XML stream.  */
void PwCustomIconV4::writeToStream(QXmlStreamWriter& xml) {
    xml.writeStartElement(XML_CUSTOM_ICON_ITEM);
    PwStreamUtilsV4::writeUuid(xml, XML_CUSTOM_ICON_ITEM_ID, uuid);
    PwStreamUtilsV4::writeBase64(xml, XML_CUSTOM_ICON_ITEM_DATA, data);
    xml.writeEndElement(); // XML_CUSTOM_ICON_ITEM
}

QString PwCustomIconV4::toString() const {
    return QString("{UUID: %1, data.size: %2}")
            .arg(uuid.toString())
            .arg(data.size());
}
/****************************/
PwMetaV4::PwMetaV4(QObject* parent) : QObject(parent) {
    clear(); // reset fields to default values
}

PwMetaV4::~PwMetaV4() {
    clear();
}

void PwMetaV4::clear() {
    QDateTime now = QDateTime::currentDateTime();

    Util::safeClear(generator);
    Util::safeClear(headerHash);
    Util::safeClear(databaseName);
    databaseNameChangedTime = now;
    Util::safeClear(databaseDescription);
    databaseDescriptionChangedTime = now;
    Util::safeClear(defaultUserName);
    defaultUserNameChangedTime = now;
    maintenanceHistoryDays = DEFAULT_MAINTENANCE_HISTORY_DAYS;
    Util::safeClear(colorString);
    masterKeyChangedTime = now;
    masterKeyChangeRec = -1;
    masterKeyChangeForce = -1;
    memoryProtection.clear();
    recycleBinEnabled = true;
    recycleBinGroupUuid.clear();
    recycleBinChangedTime = now;
    entryTemplatesGroupUuid.clear();
    entryTemplatesGroupChangedTime = now;
    historyMaxItems = DEFAULT_HISTORY_MAX_ITEMS;
    historyMaxSize = DEFAULT_HISTORY_MAX_SIZE;
    lastSelectedGroupUuid.clear();
    lastTopVisibleGroupUuid.clear();

    qDeleteAll(customIcons);
    customIcons.clear();
    //qDeleteAll(customData); - no need to, QString items are stored by value and will be deleted automagically
    customData.clear();
    qDeleteAll(binaries);
    binaries.clear();
}

ErrorCodesV4::ErrorCode PwMetaV4::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_META);

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_META == tagName))) {
        if (xml.isStartElement()) {
            if (XML_GENERATOR == tagName) {
                generator = PwStreamUtilsV4::readString(xml);
            } else if (XML_HEADER_HASH == tagName) {
                headerHash = PwStreamUtilsV4::readBase64(xml);
            } else if (XML_DATABASE_NAME == tagName) {
                databaseName = PwStreamUtilsV4::readString(xml);
            } else if (XML_DATABASE_NAME_CHANGED == tagName) {
                databaseNameChangedTime = PwStreamUtilsV4::readTime(xml);
            } else if (XML_DATABASE_DESCRIPTION == tagName) {
                databaseDescription = PwStreamUtilsV4::readString(xml);
            } else if (XML_DATABASE_DESCRIPTION_CHANGED == tagName) {
                databaseDescriptionChangedTime = PwStreamUtilsV4::readTime(xml);
            } else if (XML_DEFAULT_USERNAME == tagName) {
                defaultUserName = PwStreamUtilsV4::readString(xml);
            } else if (XML_DEFAULT_USERNAME_CHANGED == tagName) {
                defaultUserNameChangedTime = PwStreamUtilsV4::readTime(xml);
            } else if (XML_MAINTENANCE_HISTORY_DAYS == tagName) {
                maintenanceHistoryDays = PwStreamUtilsV4::readInt32(xml, DEFAULT_MAINTENANCE_HISTORY_DAYS);
            } else if (XML_COLOR == tagName) {
                colorString = PwStreamUtilsV4::readString(xml);
            } else if (XML_MASTER_KEY_CHANGED == tagName) {
                masterKeyChangedTime = PwStreamUtilsV4::readTime(xml);
            } else if (XML_MASTER_KEY_CHANGE_REC == tagName) {
                masterKeyChangeRec = PwStreamUtilsV4::readInt64(xml, -1L);
            } else if (XML_MASTER_KEY_CHANGE_FORCE == tagName) {
                masterKeyChangeForce = PwStreamUtilsV4::readInt64(xml, -1L);
            } else if (XML_MEMORY_PROTECTION == tagName) {
                ErrorCodesV4::ErrorCode err = memoryProtection.readFromStream(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (XML_CUSTOM_ICONS == tagName) {
                ErrorCodesV4::ErrorCode err = readCustomIcons(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (XML_RECYCLE_BIN_ENABLED == tagName) {
                recycleBinEnabled = PwStreamUtilsV4::readBool(xml, true);
            } else if (XML_RECYCLE_BIN_UUID == tagName) {
                recycleBinGroupUuid = PwStreamUtilsV4::readUuid(xml);
            } else if (XML_RECYCLE_BIN_CHANGED == tagName) {
                recycleBinChangedTime = PwStreamUtilsV4::readTime(xml);
            } else if (XML_ENTRY_TEMPLATES_GROUP == tagName) {
                entryTemplatesGroupUuid = PwStreamUtilsV4::readUuid(xml);
            } else if (XML_ENTRY_TEMPLATES_GROUP_CHANGED == tagName) {
                entryTemplatesGroupChangedTime = PwStreamUtilsV4::readTime(xml);
            } else if (XML_HISTORY_MAX_ITEMS == tagName) {
                historyMaxItems = PwStreamUtilsV4::readInt32(xml, -1);
            } else if (XML_HISTORY_MAX_SIZE == tagName) {
                historyMaxSize = PwStreamUtilsV4::readInt64(xml, -1);
            } else if (XML_LAST_SELECTED_GROUP == tagName) {
                lastSelectedGroupUuid = PwStreamUtilsV4::readUuid(xml);
            } else if (XML_LAST_TOP_VISIBLE_GROUP == tagName) {
                lastTopVisibleGroupUuid = PwStreamUtilsV4::readUuid(xml);
            } else if (XML_BINARIES == tagName) {
                ErrorCodesV4::ErrorCode err = readBinaries(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (XML_CUSTOM_DATA == tagName) {
                ErrorCodesV4::ErrorCode err = readCustomData(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else {
                qDebug() << "unexpected XML tag in Meta:" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_UNKNOWN_TAG_ERROR;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_PARSING_ERROR;

    debugPrint(); // TODO remove this after debug
    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwMetaV4::readCustomIcons(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_CUSTOM_ICONS);

    xml.readNext();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_CUSTOM_ICONS == xml.name()))) {
        if (xml.isStartElement()) {
            QStringRef tagName = xml.name();
            if (tagName == XML_CUSTOM_ICON_ITEM) {
                PwCustomIconV4* icon = new PwCustomIconV4(this);
                if (!icon->readFromStream(xml)) {
                    return ErrorCodesV4::XML_META_CUSTOM_ICONS_PARSING_ERROR;
                }
                customIcons.insert(icon->getUuid(), icon);
            } else {
                qDebug() << "unexpected XML tag in CustomIcons: " << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_ICONS_PARSING_ERROR;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_ICONS_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwMetaV4::readBinaries(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_BINARIES);

    xml.readNext();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_BINARIES == xml.name()))) {
        if (xml.isStartElement()) {
            QStringRef tagName = xml.name();
            if (tagName == XML_BINARY) {
                PwBinaryV4* binary = new PwBinaryV4(this);
                if (!binary->readFromStream(xml)) {
                    return ErrorCodesV4::XML_META_BINARIES_PARSING_ERROR;
                }
                binaries.insert(binary->getId(), binary);
            } else {
                qDebug() << "unexpected XML tag in Binaries: " << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_BINARIES_PARSING_ERROR;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_BINARIES_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwMetaV4::readCustomData(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_CUSTOM_DATA);

    xml.readNext();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_CUSTOM_DATA == xml.name()))) {
        if (xml.isStartElement()) {
            QStringRef tagName = xml.name();
            if (tagName == XML_CUSTOM_DATA_ITEM) {
                // read the item
                ErrorCodesV4::ErrorCode err = readCustomDataItem(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else {
                qDebug() << "unexpected XML tag in CustomData:" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwMetaV4::readCustomDataItem(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_CUSTOM_DATA_ITEM);

    QString key, value;

    xml.readNext();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_CUSTOM_DATA_ITEM == xml.name()))) {
        if (xml.isStartElement()) {
            QStringRef tagName = xml.name();
            if (tagName == XML_KEY) {
                key = PwStreamUtilsV4::readString(xml);
            } else if (tagName == XML_VALUE) {
                value = PwStreamUtilsV4::readString(xml);
            } else {
                qDebug() << "unexpected XML tag in CustomData item:" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR;

    customData.insert(key, value);
    return ErrorCodesV4::SUCCESS;
}

PwBinaryV4* PwMetaV4::getBinaryByReference(const QString& ref) const {
    if (binaries.contains(ref))
        return binaries.value(ref);
    else
        return NULL;
}

/**
 * Checks if the hashes DB file's header hash matches the one specified in Meta data, if any.
 * If no header hash was specified in Meta, returns true.
 */
bool PwMetaV4::isHeaderHashMatch(const QByteArray& dbHeaderHash) const {
    if (this->headerHash.isEmpty())
        return true;
    return (dbHeaderHash == this->headerHash);
}

ErrorCodesV4::ErrorCode PwMetaV4::writeToStream(QXmlStreamWriter& xml) {
    xml.writeStartElement(XML_META);

    PwStreamUtilsV4::writeString(xml, XML_GENERATOR, generator);
    PwStreamUtilsV4::writeBase64(xml, XML_HEADER_HASH, headerHash);
    PwStreamUtilsV4::writeString(xml, XML_DATABASE_NAME, databaseName);
    PwStreamUtilsV4::writeTime(xml, XML_DATABASE_NAME_CHANGED, databaseNameChangedTime);
    PwStreamUtilsV4::writeString(xml, XML_DATABASE_DESCRIPTION, databaseDescription);
    PwStreamUtilsV4::writeTime(xml, XML_DATABASE_DESCRIPTION_CHANGED, databaseDescriptionChangedTime);
    PwStreamUtilsV4::writeString(xml, XML_DEFAULT_USERNAME, defaultUserName);
    PwStreamUtilsV4::writeTime(xml, XML_DEFAULT_USERNAME_CHANGED, defaultUserNameChangedTime);
    PwStreamUtilsV4::writeUInt32(xml, XML_MAINTENANCE_HISTORY_DAYS, maintenanceHistoryDays);
    PwStreamUtilsV4::writeString(xml, XML_COLOR, colorString);
    PwStreamUtilsV4::writeTime(xml, XML_MASTER_KEY_CHANGED, masterKeyChangedTime);
    PwStreamUtilsV4::writeInt64(xml, XML_MASTER_KEY_CHANGE_REC, masterKeyChangeRec);
    PwStreamUtilsV4::writeInt64(xml, XML_MASTER_KEY_CHANGE_FORCE, masterKeyChangeForce);
    memoryProtection.writeToStream(xml);
    writeCustomIcons(xml);
    PwStreamUtilsV4::writeBool(xml, XML_RECYCLE_BIN_ENABLED, recycleBinEnabled);
    PwStreamUtilsV4::writeUuid(xml, XML_RECYCLE_BIN_UUID, recycleBinGroupUuid);
    PwStreamUtilsV4::writeTime(xml, XML_RECYCLE_BIN_CHANGED, recycleBinChangedTime);
    PwStreamUtilsV4::writeUuid(xml, XML_ENTRY_TEMPLATES_GROUP, entryTemplatesGroupUuid);
    PwStreamUtilsV4::writeTime(xml, XML_ENTRY_TEMPLATES_GROUP_CHANGED, entryTemplatesGroupChangedTime);
    PwStreamUtilsV4::writeInt32(xml, XML_HISTORY_MAX_ITEMS, historyMaxItems);
    PwStreamUtilsV4::writeInt64(xml, XML_HISTORY_MAX_SIZE, historyMaxSize);
    PwStreamUtilsV4::writeUuid(xml, XML_LAST_SELECTED_GROUP, lastSelectedGroupUuid);
    PwStreamUtilsV4::writeUuid(xml, XML_LAST_TOP_VISIBLE_GROUP, lastTopVisibleGroupUuid);
    writeBinaries(xml);
    writeCustomData(xml);

    xml.writeEndElement(); //XML_META
    return ErrorCodesV4::SUCCESS;
}

void PwMetaV4::writeCustomIcons(QXmlStreamWriter& xml) const {
    if (customIcons.size() > 0) {
        xml.writeStartElement(XML_CUSTOM_ICONS);
        QListIterator<PwCustomIconV4*> iter(customIcons.values());
        while (iter.hasNext()) {
            iter.next()->writeToStream(xml);
        }
        xml.writeEndElement(); // XML_CUSTOM_ICONS
    } else {
        // do not write anything
    }
}

void PwMetaV4::writeBinaries(QXmlStreamWriter& xml) const {
    if (binaries.size() > 0) {
        xml.writeStartElement(XML_BINARIES);
        QMapIterator<QString, PwBinaryV4*> iter(binaries);
        while (iter.hasNext()) {
            iter.next().value()->writeToStream(xml);
        }
        xml.writeEndElement(); // XML_BINARIES
    } else {
        // do not write anything
    }
}

void PwMetaV4::writeCustomData(QXmlStreamWriter& xml) const {
    if (customData.size() > 0) {
        xml.writeStartElement(XML_CUSTOM_DATA);
        QMapIterator<QString, QString> iter(customData);
        while (iter.hasNext()) {
            iter.next();
            PwStreamUtilsV4::writeString(xml, XML_KEY, iter.key());
            PwStreamUtilsV4::writeString(xml, XML_VALUE, iter.value());
        }
        xml.writeEndElement(); // XML_CUSTOM_DATA
    } else {
        xml.writeEmptyElement(XML_CUSTOM_DATA);
    }
}

void PwMetaV4::debugPrint() const {
    qDebug() << "Meta header:";
    qDebug() << "  generator:" << generator;
    qDebug() << "  databaseName:" << databaseName;
    qDebug() << "  databaseNameChangedTime:" << databaseNameChangedTime;
    qDebug() << "  databaseDescription:" << databaseDescription;
    qDebug() << "  databaseDescriptionChangedTime:" << databaseDescriptionChangedTime;
    qDebug() << "  defaultUserName:" << defaultUserName;
    qDebug() << "  defaultUserNameChangedTime:" << defaultUserNameChangedTime;
    qDebug() << "  maintenanceHistoryDays:" << maintenanceHistoryDays;
    qDebug() << "  colorString:" << colorString;
    qDebug() << "  masterKeyChangedTime:" << masterKeyChangedTime;
    qDebug() << "  masterKeyChangeRec:" << masterKeyChangeRec;
    qDebug() << "  masterKeyChangeForce:" << masterKeyChangeForce;
    qDebug() << "  memoryProtection:" << memoryProtection.toString();
    qDebug() << "  customIcons.size():" << customIcons.size();
    foreach (PwCustomIconV4* it, customIcons.values()) {
        qDebug() << "    " << it->toString();
    }
    qDebug() << "  recycleBinEnabled:" << recycleBinEnabled;
    qDebug() << "  recycleBinGroupUuid:" << recycleBinGroupUuid.toString();
    qDebug() << "  recycleBinChangedTime:" << recycleBinChangedTime;
    qDebug() << "  entryTemplatesGroupUuid:" << entryTemplatesGroupUuid.toString();
    qDebug() << "  entryTemplatesGroupChangedTime:" << entryTemplatesGroupChangedTime;
    qDebug() << "  historyMaxItems:" << historyMaxItems;
    qDebug() << "  historyMaxSize:" << historyMaxSize;
    qDebug() << "  lastSelectedGroupUuid:" << lastSelectedGroupUuid.toString();
    qDebug() << "  lastTopVisibleGroupUuid:" << lastTopVisibleGroupUuid.toString();
    qDebug() << "  customData.size():" << customData.size();
    qDebug() << "  binaries.size():" << binaries.size();
    foreach (PwBinaryV4* it, binaries.values()) {
        qDebug() << "    " << it->toString();
    }
    qDebug() << "--- end of Meta ---";
}
