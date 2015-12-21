/*
 * PwMetaV4.cpp
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */


#include "db/v4/PwMetaV4.h"
#include "db/PwUuid.h"
#include "util/Util.h"
#include "db/v4/PwGroupV4.h"
#include "db/v4/PwEntryV4.h"
#include "db/v4/PwStreamUtilsV4.h"


// Some database defaults
int DEFAULT_HISTORY_MAX_ITEMS = 10; // -1 for unlimited
int DEFAULT_HISTORY_MAX_SIZE = 6 * 1024 * 1024; // -1 for unlimited
int DEFAULT_MAINTENANCE_HISTORY_DAYS = 365;
const QString GENERATOR_NAME = QString("KeePassB"); // short app name, no version

MemoryProtection::MemoryProtection(QObject* parent) : QObject(parent) {
    clear(); // reset to default values
}

MemoryProtection::~MemoryProtection() {
    // empty
}

void MemoryProtection::clear() {
    _protectTitle = false;
    _protectUserName = false;
    _protectPassword = true;
    _protectUrl = false;
    _protectNotes = false;
}

QString MemoryProtection::toString() const {
    return QString("{title: %1, username: %2, password: %3, url: %4, notes: %5}")
            .arg(_protectTitle)
            .arg(_protectUserName)
            .arg(_protectPassword)
            .arg(_protectUrl)
            .arg(_protectNotes);
}

ErrorCodesV4::ErrorCode MemoryProtection::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_MEMORY_PROTECTION);

    clear();

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_MEMORY_PROTECTION == tagName))) {
        if (xml.isStartElement()) {
            if (XML_MEMORY_PROTECTION_PROTECT_TITLE == tagName) {
                _protectTitle = PwStreamUtilsV4::readBool(xml, false);
            } else if (XML_MEMORY_PROTECTION_PROTECT_USERNAME == tagName) {
                _protectUserName = PwStreamUtilsV4::readBool(xml, false);
            } else if (XML_MEMORY_PROTECTION_PROTECT_PASSWORD == tagName) {
                _protectPassword = PwStreamUtilsV4::readBool(xml, true);
            } else if (XML_MEMORY_PROTECTION_PROTECT_URL == tagName) {
                _protectUrl = PwStreamUtilsV4::readBool(xml, false);
            } else if (XML_MEMORY_PROTECTION_PROTECT_NOTES == tagName) {
                _protectNotes = PwStreamUtilsV4::readBool(xml, false);
            } else {
                LOG("WARN: unknown MemoryProtection tag: %s", tagName.toUtf8().constData());
                return ErrorCodesV4::XML_META_MEMORY_PROTECTION_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_MEMORY_PROTECTION_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;
}

void MemoryProtection::writeToStream(QXmlStreamWriter& xml) {
    xml.writeStartElement(XML_MEMORY_PROTECTION);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_TITLE, _protectTitle);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_USERNAME, _protectUserName);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_PASSWORD, _protectPassword);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_URL, _protectUrl);
    PwStreamUtilsV4::writeBool(xml, XML_MEMORY_PROTECTION_PROTECT_NOTES, _protectNotes);
    xml.writeEndElement(); // XML_MEMORY_PROTECTION
}

/****************************/
PwBinaryV4::PwBinaryV4(QObject* parent) : QObject(parent) {
    // nothing to do
}
PwBinaryV4::PwBinaryV4(QObject* parent, const int id, const QByteArray& data, const bool isCompressed, bool isProtected) :
            QObject(parent), _id(id), _data(Util::deepCopy(data)),
            _isCompressed(isCompressed), _isProtected(isProtected) {
    //nothing else to do
}
PwBinaryV4::~PwBinaryV4() {
    clear();
}
void PwBinaryV4::clear() {
    _id = -1;
    _isCompressed = false;
    _isProtected = false;
    Util::safeClear(_data);
}

ErrorCodesV4::ErrorCode PwBinaryV4::readFromStream(QXmlStreamReader& xml, Salsa20& salsa20) {
    Q_ASSERT(xml.name() == XML_BINARY);

    clear();

    if (xml.isStartElement() && (xml.name() == XML_BINARY)) {
        QXmlStreamAttributes attrs = xml.attributes();
        bool convOk;
        _id = attrs.value(XML_BINARY_ID).toString().toInt(&convOk);
        if (!convOk) {
            LOG("PwBinaryV4::readFromStream() int conversion failed");
            // We cannot fix this, so fail
            _id = -1;
        }
        _isCompressed = (attrs.value(XML_BINARY_COMPRESSED) == XML_TRUE);
        _isProtected = (attrs.value(XML_PROTECTED) == XML_TRUE);
        _data = PwStreamUtilsV4::readBase64(xml); // implicit deep copy
        // data might probably be empty
        if (_isProtected) {
            // remove protection but keep the flag for saving
            salsa20.xorWithNextBytes(_data);
        }
    } else {
        LOG("invalid Binary structure, got %s tag.", xml.name().toUtf8().constData());
        return ErrorCodesV4::XML_META_BINARY_PARSING_ERROR_TAG;
    }

    if (_id < 0)
        return ErrorCodesV4::XML_META_BINARY_PARSING_ERROR_1;

    if (xml.hasError())
        return ErrorCodesV4::XML_META_BINARY_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;

}

void PwBinaryV4::writeToStream(QXmlStreamWriter& xml, Salsa20& salsa20) {
    xml.writeStartElement(XML_BINARY);
    xml.writeAttribute(XML_BINARY_ID, QString::number(_id));
    xml.writeAttribute(XML_BINARY_COMPRESSED, _isCompressed ? XML_TRUE : XML_FALSE);
    if (_isProtected) {
        xml.writeAttribute(XML_PROTECTED, XML_TRUE);
        QByteArray protectedData = Util::deepCopy(_data);
        salsa20.xorWithNextBytes(protectedData);
        xml.writeCharacters(protectedData.toBase64());
        Util::safeClear(protectedData);
    } else {
        xml.writeCharacters(_data.toBase64());
    }
    xml.writeEndElement(); // XML_BINARY
}

QString PwBinaryV4::toString() const {
    return QString("{ID: %1, compressed: %2, protected: %3, data.size: %4}")
            .arg(_id).arg(_isCompressed).arg(_isProtected).arg(_data.size());
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

ErrorCodesV4::ErrorCode PwCustomIconV4::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(xml.name() == XML_CUSTOM_ICON_ITEM);

    clear();

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_CUSTOM_ICON_ITEM == tagName))) {
        if (xml.isStartElement()) {
            if (tagName == XML_CUSTOM_ICON_ITEM_ID) {
                uuid = PwStreamUtilsV4::readUuid(xml);
            } else if (tagName == XML_CUSTOM_ICON_ITEM_DATA) {
                data = PwStreamUtilsV4::readBase64(xml); // implicit deep copy
            } else {
                LOG("unexpected XML tag in CustomIcon: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_ICON_ITEM_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    if (data.isEmpty())
        return ErrorCodesV4::XML_META_CUSTOM_ICON_ITEM_PARSING_ERROR_1;

    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_ICON_ITEM_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;
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

void PwMetaV4::setRecycleBinGroupUuid(const PwUuid& uuid) {
    if (recycleBinGroupUuid != uuid) {
        recycleBinGroupUuid = uuid;
        setRecycleBinChangedTime(QDateTime::currentDateTime());
    }
}

void PwMetaV4::setRecycleBinChangedTime(const QDateTime& time) {
    recycleBinChangedTime = time;
}

void PwMetaV4::setMasterKeyChangedTime(const QDateTime& time) {
    masterKeyChangedTime = time;
}

ErrorCodesV4::ErrorCode PwMetaV4::readFromStream(QXmlStreamReader& xml, Salsa20& salsa20) {
    Q_ASSERT(xml.name() == XML_META);

    clear();

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
                ErrorCodesV4::ErrorCode err = readBinaries(xml, salsa20);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (XML_CUSTOM_DATA == tagName) {
                ErrorCodesV4::ErrorCode err = readCustomData(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else {
                LOG("unexpected XML tag in Meta: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_PARSING_ERROR_GENERIC;

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
                ErrorCodesV4::ErrorCode err = icon->readFromStream(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
                customIcons.insert(icon->getUuid(), icon);
            } else {
                LOG("unexpected XML tag in CustomIcons: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_ICONS_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_ICONS_PARSING_ERROR_GENERIC;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwMetaV4::readBinaries(QXmlStreamReader& xml, Salsa20& salsa20) {
    Q_ASSERT(xml.name() == XML_BINARIES);

    xml.readNext();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_BINARIES == xml.name()))) {
        if (xml.isStartElement()) {
            QStringRef tagName = xml.name();
            if (tagName == XML_BINARY) {
                PwBinaryV4* binary = new PwBinaryV4(this);
                ErrorCodesV4::ErrorCode err = binary->readFromStream(xml, salsa20);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
                binaries.insert(binary->getId(), binary);
            } else {
                LOG("unexpected XML tag in Binaries: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_BINARIES_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_BINARIES_PARSING_ERROR_GENERIC;

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
                LOG("unexpected XML tag in CustomData: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_DATA_PARSING_ERROR_GENERIC;

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
                LOG("unexpected XML tag in CustomData item: %s", tagName.toUtf8().constData());
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_META_CUSTOM_DATA_ITEM_PARSING_ERROR_TAG;
            }
        }
        xml.readNext();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_META_CUSTOM_DATA_ITEM_PARSING_ERROR_GENERIC;

    customData.insert(key, value);
    return ErrorCodesV4::SUCCESS;
}

PwBinaryV4* PwMetaV4::getBinaryById(const int id) const {
    if (binaries.contains(id))
        return binaries.value(id);
    else
        return NULL;
}

/**
 * Updates the list of binaries by traversing all the entries and their histories.
 * Previously stored binaries are cleared.
 */
void PwMetaV4::updateBinaries(PwGroupV4* root) {
    Q_ASSERT(root != NULL);

    QList<PwGroup*> allGroups;
    QList<PwEntry*> allEntries;
    root->getAllChildren(allGroups, allEntries);

    qDeleteAll(binaries);
    binaries.clear();

    QListIterator<PwEntry*> iter(allEntries);
    while (iter.hasNext()) {
        PwEntryV4* entry = dynamic_cast<PwEntryV4*>(iter.next());
        updateBinaries(entry);
    }
}

/**
 * Adds entry's attachments to the binary pool and updates attachment referenceIDs accordingly.
 * Also looks into entry's history.
 */
void PwMetaV4::updateBinaries(PwEntryV4* entry) {
    Q_ASSERT(entry != NULL);

    // Process previous versions of the entry, if any
    QListDataModel<PwEntryV4*>* historyModel = dynamic_cast<QListDataModel<PwEntryV4*>*>(entry->getHistoryDataModel());
    for (int i = 0; i < historyModel->size(); i++) {
        PwEntryV4* historyEntry = historyModel->value(i);
        updateBinaries(historyEntry);
    }

    // Process the entry itself
    PwAttachmentDataModel* dataModel = entry->getAttachmentsDataModel();
    for (int i = 0; i < dataModel->size(); i++) {
        PwAttachment* att = dataModel->value(i);

        // was the attachment already added to the binary pool?
        PwBinaryV4* binary = findBinary(att->getData(), att->isCompressed());
        if (!binary) {
            int nextId = binaries.size();
            // create new item in Meta's binary pool
            binary = new PwBinaryV4(this, nextId, att->getData(), att->isCompressed());
            binaries.insert(nextId, binary);
        }
        // update attachment's reference to the binary
        att->setId(binary->getId());
    }
}

/**
 * Finds the binary with given data/compression flag within the binary pool.
 * Returns NULL if nothing found.
 */
PwBinaryV4* PwMetaV4::findBinary(const QByteArray& data, bool isCompressed) const {
    QMapIterator<int, PwBinaryV4*> iter(binaries);
    while (iter.hasNext()) {
        PwBinaryV4* binary = iter.next().value();
        if ((binary->isCompressed() == isCompressed) && (binary->getData() == data))
            return binary;
    }
    return NULL;
}

void PwMetaV4::setHeaderHash(const QByteArray& headerHash) {
    if (this->headerHash != headerHash) {
        this->headerHash = Util::deepCopy(headerHash);
    }
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

ErrorCodesV4::ErrorCode PwMetaV4::writeToStream(QXmlStreamWriter& xml, Salsa20& salsa20) {
    xml.writeStartElement(XML_META);

    // Replace the original generator name with this app's name
    generator = GENERATOR_NAME;
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
    writeBinaries(xml, salsa20);
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

void PwMetaV4::writeBinaries(QXmlStreamWriter& xml, Salsa20& salsa20) const {
    if (binaries.size() > 0) {
        xml.writeStartElement(XML_BINARIES);
        QMapIterator<int, PwBinaryV4*> iter(binaries);
        while (iter.hasNext()) {
            iter.next().value()->writeToStream(xml, salsa20);
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
            xml.writeStartElement(XML_CUSTOM_DATA_ITEM);
            PwStreamUtilsV4::writeString(xml, XML_KEY, iter.key());
            PwStreamUtilsV4::writeString(xml, XML_VALUE, iter.value());
            xml.writeEndElement(); // XML_CUSTOM_DATA_ITEM
        }
        xml.writeEndElement(); // XML_CUSTOM_DATA
    } else {
        xml.writeEmptyElement(XML_CUSTOM_DATA);
    }
}
