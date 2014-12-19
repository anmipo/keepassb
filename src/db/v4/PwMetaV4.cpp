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
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    return (!xml.hasError() && !data.isEmpty());
}

QString PwCustomIconV4::toString() const {
    return QString("{UUID: %1, data.size: %2}")
            .arg(uuid.toString())
            .arg(data.size());
}
/****************************/
PwDatabaseV4Meta::PwDatabaseV4Meta(QObject* parent) : QObject(parent) {
    clear(); // reset fields to default values
}

PwDatabaseV4Meta::~PwDatabaseV4Meta() {
    clear();
}

void PwDatabaseV4Meta::clear() {
    QDateTime now = QDateTime::currentDateTime();

    Util::safeClear(generator);
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

ErrorCodesV4::ErrorCode PwDatabaseV4Meta::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_META);

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_META == tagName))) {
        if (xml.isStartElement()) {
            if (XML_GENERATOR == tagName) {
                generator = PwStreamUtilsV4::readString(xml);
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
                //PwStreamUtilsV4::readUnknown(xml);
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

ErrorCodesV4::ErrorCode PwDatabaseV4Meta::readCustomIcons(QXmlStreamReader& xml) {
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
                //PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_ICONS_PARSING_ERROR;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_ICONS_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4Meta::readBinaries(QXmlStreamReader& xml) {
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
                //PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_BINARIES_PARSING_ERROR;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_BINARIES_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4Meta::readCustomData(QXmlStreamReader& xml) {
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
                //PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4Meta::readCustomDataItem(QXmlStreamReader& xml) {
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
                //PwStreamUtilsV4::readUnknown(xml);
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

PwBinaryV4* PwDatabaseV4Meta::getBinaryByReference(const QString& ref) const {
    if (binaries.contains(ref))
        return binaries.value(ref);
    else
        return NULL;
}

void PwDatabaseV4Meta::debugPrint() const {
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
