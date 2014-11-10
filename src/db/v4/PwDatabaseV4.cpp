/*
 * PwDatabaseV4.cpp
 *
 *  Created on: 1 Jun 2014
 *      Author: Andrei Popleteev
 */

#include "PwDatabaseV4.h"

#include <QObject>
#include <QDataStream>
#include "CryptoManager.h"
#include "sbreturn.h"
#include "husha2.h"
#include "huaes.h"
#include "db/PwUuid.h"
#include "util/Util.h"
#include "db/v4/PwStreamUtilsV4.h"

// KeePass2 XML tag names
const QString XML_META = QString("Meta");
const QString XML_ROOT = QString("Root");
const QString XML_GROUP = QString("Group");
const QString XML_ENTRY = QString("Entry");

const QString XML_GENERATOR                       = QString("Generator");
const QString XML_DATABASE_NAME                   = QString("DatabaseName");
const QString XML_DATABASE_NAME_CHANGED           = QString("DatabaseNameChanged");
const QString XML_DATABASE_DESCRIPTION            = QString("DatabaseDescription");
const QString XML_DATABASE_DESCRIPTION_CHANGED    = QString("DatabaseDescriptionChanged");
const QString XML_DEFAULT_USERNAME                = QString("DefaultUserName");
const QString XML_DEFAULT_USERNAME_CHANGED        = QString("DefaultUserNameChanged");
const QString XML_MAINTENANCE_HISTORY_DAYS        = QString("MaintenanceHistoryDays");
const QString XML_COLOR                           = QString("Color");
const QString XML_MASTER_KEY_CHANGED              = QString("MasterKeyChanged");
const QString XML_MASTER_KEY_CHANGE_REC           = QString("MasterKeyChangeRec");
const QString XML_MASTER_KEY_CHANGE_FORCE         = QString("MasterKeyChangeForce");
const QString XML_MEMORY_PROTECTION               = QString("MemoryProtection");
const QString XML_MEMORY_PROTECTION_PROTECT_TITLE = QString("ProtectTitle");
const QString XML_MEMORY_PROTECTION_PROTECT_USERNAME = QString("ProtectUserName");
const QString XML_MEMORY_PROTECTION_PROTECT_PASSWORD = QString("ProtectPassword");
const QString XML_MEMORY_PROTECTION_PROTECT_URL   = QString("ProtectURL");
const QString XML_MEMORY_PROTECTION_PROTECT_NOTES = QString("ProtectNotes");
const QString XML_RECYCLE_BIN_ENABLED             = QString("RecycleBinEnabled");
const QString XML_RECYCLE_BIN_UUID                = QString("RecycleBinUUID");
const QString XML_RECYCLE_BIN_CHANGED             = QString("RecycleBinChanged");
const QString XML_ENTRY_TEMPLATES_GROUP           = QString("EntryTemplatesGroup");
const QString XML_ENTRY_TEMPLATES_GROUP_CHANGED   = QString("EntryTemplatesGroupChanged");
const QString XML_HISTORY_MAX_ITEMS               = QString("HistoryMaxItems");
const QString XML_HISTORY_MAX_SIZE                = QString("HistoryMaxSize");
const QString XML_LAST_SELECTED_GROUP             = QString("LastSelectedGroup");
const QString XML_LAST_TOP_VISIBLE_GROUP          = QString("LastTopVisibleGroup");
const QString XML_CUSTOM_ICONS                    = QString("CustomIcons");
const QString XML_CUSTOM_ICON_ITEM                = QString("Icon");
const QString XML_CUSTOM_ICON_ITEM_ID             = QString("UUID");
const QString XML_CUSTOM_ICON_ITEM_DATA           = QString("Data");
const QString XML_BINARIES                        = QString("Binaries");
const QString XML_BINARY_ID                       = QString("ID");
const QString XML_BINARY_COMPRESSED               = QString("Compressed");
const QString XML_CUSTOM_DATA                     = QString("CustomData");
const QString XML_CUSTOM_DATA_ITEM                = QString("Item");
const QString XML_BINARY                          = QString("Binary");

const QString XML_TITLE = QString("Title");
const QString XML_USERNAME = QString("UserName");
const QString XML_PASSWORD = QString("Password");
const QString XML_URL = QString("URL");
const QString XML_NOTES = QString("Notes");

const QString XML_UUID = QString("UUID");
const QString XML_NAME = QString("Name");
const QString XML_ICON_ID = QString("IconID");
const QString XML_STRING = QString("String");
const QString XML_HISTORY = QString("History");
const QString XML_KEY = QString("Key");
const QString XML_VALUE = QString("Value");
const QString XML_PROTECTED = QString("Protected");
const QString XML_TRUE = QString("True"); // Since Qt/Cascades does not have string-to-bool conversion, we need this
const QString XML_REF = QString("Ref");

const QString XML_TIMES = QString("Times");
const QString XML_LAST_MODIFICATION_TIME = QString("LastModificationTime");
const QString XML_CREATION_TIME = QString("CreationTime");
const QString XML_LAST_ACCESS_TIME = QString("LastAccessTime");
const QString XML_EXPIRY_TIME = QString("ExpiryTime");
const QString XML_EXPIRES = QString("Expires");

// Tag names for XML-formatted key files
const QString XML_KEYFILE = "KeyFile";
const QString XML_KEYFILE_META = "Meta";
const QString XML_KEYFILE_KEY = "Key";
const QString XML_KEYFILE_DATA = "Data";

// Some database defaults
int DEFAULT_HISTORY_MAX_ITEMS = 10; // -1 for unlimited
int DEFAULT_HISTORY_MAX_SIZE = 6 * 1024 * 1024; // -1 for unlimited
int DEFAULT_MAINTENANCE_HISTORY_DAYS = 365;

// Cypher parameters and signatures
const QByteArray SALSA_20_ID = QByteArray("\x02\x00\x00\x00", 4);
const QByteArray AES_ID      = QByteArray("\x31\xC1\xF2\xE6\xBF\x71\x43\x50\xBE\x58\x05\x21\x6A\xFC\x5A\xFF", 16);
const QByteArray SALSA_20_INIT_VECTOR = QByteArray("\xE8\x30\x09\x4B\x97\x20\x5D\x2A");


// DB unlock stages progress percentage
const int UNLOCK_PROGRESS_INIT = 0;
const int UNLOCK_PROGRESS_HEADER_READ = 5;
const int UNLOCK_PROGRESS_KEY_TRANSFORMED = 70;
const int UNLOCK_PROGRESS_DECRYPTED = 80;
const int UNLOCK_PROGRESS_BLOCKS_READ = 90;
const int UNLOCK_PROGRESS_UNPACKED = 95;
const int UNLOCK_PROGRESS_DONE = 100;


PwHeaderV4::PwHeaderV4(QObject* parent) : QObject(parent), data() {
    initialized = false;
    transformRounds = 0;
    size = 0;
}

PwHeaderV4::~PwHeaderV4() {
    clear();
}

PwHeaderV4::ErrorCode PwHeaderV4::read(QDataStream& stream) {
    Q_ASSERT(!initialized);
    clear();

    // check file signatures (although probably checked before)
    quint32 sign1, sign2, fileVersion;
    stream >> sign1 >> sign2 >> fileVersion;
    size += 3 * sizeof(quint32);
    if (sign1 !=  SIGNATURE_1)
        return SIGNATURE_1_MISMATCH;
    if (sign2 != SIGNATURE_2)
        return SIGNATURE_2_MISMATCH;
    if ((fileVersion & FILE_VERSION_CRITICAL_MASK) != (FILE_VERSION & FILE_VERSION_CRITICAL_MASK))
        return UNSUPPORTED_FILE_VERSION;

    qDebug("Signatures match");

    // read header fields
    quint8 fieldId;
    quint16 fSize;
    int fieldSize;
    while (true) {
        stream >> fieldId >> fSize;
        fieldSize = fSize;
        size += sizeof(fieldId) + sizeof(fSize) + fieldSize;

        if (fieldId == HEADER_END) {
            initialized = true;
            break;
        }

        char buf[fieldSize];
        stream.readRawData(buf, fieldSize);

        QByteArray fieldValue (buf, fieldSize);
        switch (fieldId) {
        case HEADER_COMMENT:
            break;
        case HEADER_CIPHER_ID:
            if (fieldValue != AES_ID)
                return NOT_AES; // Not AES cypher
            break;
        case HEADER_COMPRESSION_FLAGS:
            if (fieldValue.at(0) > 1)
                return UNKNOWN_COMPRESSION_ALGORITHM;
            break;
        case HEADER_MASTER_SEED:
            if (fieldSize != SB_SHA256_DIGEST_LEN)
                return TRANSFORM_SEED_SIZE_MISMATCH; // Master seed size is not 32 bytes
            break;
        case HEADER_TRANSFORM_SEED:
            if (fieldSize != SB_SHA256_DIGEST_LEN)
                return TRANSFORM_SEED_SIZE_MISMATCH; // Transform seed size is not 32 bytes
            break;
        case HEADER_TRANSFORM_ROUNDS:
            transformRounds = *(quint64*)fieldValue.constData();
            break;
        case HEADER_ENCRYPTION_IV:
            if (fieldSize != 16)
                return INITIAL_VECTOR_SIZE_MISMATCH; // Initial vector size is not 16 bytes
            break;
        case HEADER_PROTECTED_STREAM_KEY:
            if (fieldSize != SB_SHA256_DIGEST_LEN)
                return PROTECTED_STREAM_SIZE_MISMATCH; // Hashed protected stream key size is not 32 bytes
            break;
        case HEADER_STREAM_START_BYTES:
            break;
        case HEADER_INNER_RANDOM_STREAM_ID:
            if (fieldValue != SALSA_20_ID)
                return NOT_SALSA20; // Not Salsa20 algorithm
            break;
        //case HEADER_END: has been processed above
        default:
            qDebug() << "Strange header ID: " << fieldId;
        }
        qDebug() << "Header" << fieldId << "Value " << fieldValue.toHex();
        data.insert(fieldId, fieldValue);
    }
    return SUCCESS;
}

QString PwHeaderV4::getErrorMessage(ErrorCode errCode) {
    switch (errCode) {
    case SUCCESS:
        return "";
    case SIGNATURE_1_MISMATCH:
        // fallthrough
    case SIGNATURE_2_MISMATCH:
        return tr("Wrong database signature", "Error message when opening a database.");
    case UNSUPPORTED_FILE_VERSION:
        return tr("Unsupported database version", "Error message when opening a database.");
    case UNKNOWN_COMPRESSION_ALGORITHM:
        return tr("Unknown compression algorithm", "Error message when opening a database.");
    case NOT_AES:
        return tr("Twofish cypher is not supported", "Error message when opening a database. 'Twofish' is an algorithm name, do not translate it.");
    default:
        return tr("Header error", "Error message when opening a database. 'Header' refers to supplemental data placed at the beginning of a file.");
    }
}

void PwHeaderV4::clear() {
    data.clear();
    size = 0;
    transformRounds = 0;
    initialized = false;
}

quint64 PwHeaderV4::getTransformRounds() const {
    return transformRounds;
}

QByteArray PwHeaderV4::getTransformSeed() const {
    return data.value(HEADER_TRANSFORM_SEED);
}

QByteArray PwHeaderV4::getMasterSeed() const {
    return data.value(HEADER_MASTER_SEED);
}

QByteArray PwHeaderV4::getInitialVector() const {
    return data.value(HEADER_ENCRYPTION_IV);
}

QByteArray PwHeaderV4::getStreamStartBytes() const {
    return data.value(HEADER_STREAM_START_BYTES);
}

QByteArray PwHeaderV4::getProtectedStreamKey() const {
    return data.value(HEADER_PROTECTED_STREAM_KEY);
}

bool PwHeaderV4::isCompressed() const {
    return (data.value(HEADER_COMPRESSION_FLAGS).at(0) != 0);
}

int PwHeaderV4::sizeInBytes() const {
    return size;
}


/****************************/
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


const PwUuid& PwDatabaseV4Meta::getRecycleBinGroupUuid() const {
    return recycleBinGroupUuid;
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
/****************************/

PwDatabaseV4::PwDatabaseV4(QObject* parent) :
        PwDatabase(parent),
        header(),
        meta(),
        combinedKey(SB_SHA256_DIGEST_LEN, 0),
        aesKey(SB_SHA256_DIGEST_LEN, 0),
        salsa20() {
    header.setParent(this);
    meta.setParent(this);
}

PwDatabaseV4::~PwDatabaseV4() {
    // nothing to do here
}

/**
 * Returns the Backup group of this database.
 * If createIfMissing is true, creates the group if it is missing.
 * (However, if backup is disabled will not create anything and still return NULL).
 */
PwGroup* PwDatabaseV4::getBackupGroup(bool createIfMissing) {
    // TODO implement PwDatabaseV4::getBackupGroup
    Q_UNUSED(createIfMissing); // remove when method is implemented
    return NULL;
}

bool PwDatabaseV4::isSignatureMatch(const QByteArray& rawDbData) {
    QDataStream stream (rawDbData);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint32 sign1, sign2;
    stream >> sign1 >> sign2;

    return (sign1 ==  PwHeaderV4::SIGNATURE_1) && (sign2 == PwHeaderV4::SIGNATURE_2);
}

void PwDatabaseV4::clear() {
    header.clear();
    meta.clear();
    Util::safeClear(combinedKey);
    Util::safeClear(aesKey);
    PwDatabase::clear(); // ancestor's cleaning
}

void PwDatabaseV4::load(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) {
    if (!buildCompositeKey(password.toUtf8(), keyFileData, combinedKey)) {
        emit dbLoadError(tr("Cryptographic library error", "Generic error message from a cryptographic library"), COMPOSITE_KEY_ERROR);
        return;
    }

    if (readDatabase(dbFileData))
        emit dbUnlocked();
}

/**
 * Extracts the key from a correctly-formed XML file.
 * Returns true if successful, false otherwise.
 */
bool PwDatabaseV4::processXmlKeyFile(const QByteArray& keyFileData, QByteArray& key) const {
    QXmlStreamReader xml(keyFileData);
    if (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE)) {
            if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE_META)) {
                xml.skipCurrentElement(); // skip the Meta element
                if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE_KEY)) {
                    if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE_DATA)) {
                        // QString keyText = xml.readElementText();
                        // key = QByteArray::fromBase64(keyText.toLatin1());
                        key = PwStreamUtilsV4::readBase64(xml);
                        if (!xml.hasError()) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    xml.clear();
    return false;
}

bool PwDatabaseV4::buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const {
    CryptoManager* cm = CryptoManager::instance();

    QByteArray preKey;
    int ec;
    // if no key file were supplied, the keyFileData will be empty
    if (!passwordKey.isEmpty() && !keyFileData.isEmpty()) {
        qDebug() << "using password and key file";
        ec = cm->sha256(passwordKey, preKey);
        if (ec != SB_SUCCESS)
            return false;

        QByteArray fKey;
        if (!processKeyFile(keyFileData, fKey))
            return false;
        preKey.append(fKey);
    } else if (keyFileData.isEmpty()) {
        qDebug() << "using password only";
        int ec = cm->sha256(passwordKey, preKey);
        if (ec != SB_SUCCESS)
            return false;
    } else if (passwordKey.isEmpty()) {
        qDebug() << "using key file only";
        if (!processKeyFile(keyFileData, preKey))
            return false;
    } else {
        qDebug() << "empty keys provided (should not happen)";
        return false;
    }

    ec = cm->sha256(preKey, combinedKey);
    Util::safeClear(preKey);
    if (ec != SB_SUCCESS)
        return false;

    return true;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::transformKey(const PwHeaderV4& header, const QByteArray& combinedKey, QByteArray& aesKey,
        const int progressFrom, const int progressTo) {
//    aesKey.clear();

    CryptoManager* cm = CryptoManager::instance();

    QByteArray subKey1 = combinedKey.left(16);
    QByteArray subKey2 = combinedKey.right(16);
    QByteArray key = header.getTransformSeed();
    quint64 transformRounds = header.getTransformRounds();

    // temporary arrays for storing intermediate keys
    QByteArray subKey1bis(subKey1.length(), 0);
    QByteArray subKey2bis(subKey2.length(), 0);

    int progress = progressFrom;
    int subProgress = 0;
    int subProgressThreshold = ceil(transformRounds / (progressTo - progressFrom));
    int ec;

    // prepare key transform
    if (cm->beginKeyTransform(key, SB_AES_128_KEY_BYTES) != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_INIT_ERROR;

    unsigned char* origKey1 = reinterpret_cast<unsigned char*>(subKey1.data());
    unsigned char* origKey2 = reinterpret_cast<unsigned char*>(subKey2.data());
    unsigned char* transKey1 = reinterpret_cast<unsigned char*>(subKey1bis.data());
    unsigned char* transKey2 = reinterpret_cast<unsigned char*>(subKey2bis.data());
    for (quint64 round = 0; round < transformRounds; round++) {
        ec = cm->performKeyTransform(origKey1, transKey1);
        memcpy(origKey1, transKey1, SB_AES_128_BLOCK_BYTES);
        if (ec != SB_SUCCESS) break;

        ec = cm->performKeyTransform(origKey2, transKey2);
        memcpy(origKey2, transKey2, SB_AES_128_BLOCK_BYTES);
        if (ec != SB_SUCCESS) break;

        if (++subProgress > subProgressThreshold) {
            subProgress = 0;
            progress++;
            emit progressChanged(progress);
        }
    }
    if (ec != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_ERROR_1;

    if (cm->endKeyTransform() != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_END_ERROR;


    QByteArray step2key;
    step2key.append(subKey1);
    step2key.append(subKey2);
    QByteArray transformedKey;
    ec = cm->sha256(step2key, transformedKey);
    Util::safeClear(subKey1);
    Util::safeClear(subKey2);
    Util::safeClear(step2key);
    if (ec != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_ERROR_2;

    QByteArray step3key(header.getMasterSeed());
    step3key.append(transformedKey);
    ec = cm->sha256(step3key, aesKey);
    Util::safeClear(transformedKey);
    Util::safeClear(step3key);
    if (ec != SB_SUCCESS)
        return ErrorCodesV4::KEY_TRANSFORM_ERROR_3;

    return ErrorCodesV4::SUCCESS;
}

bool PwDatabaseV4::readDatabase(const QByteArray& dbBytes) {
    QDataStream stream (dbBytes);
    stream.setByteOrder(QDataStream::LittleEndian);

    emit progressChanged(UNLOCK_PROGRESS_INIT);

    PwHeaderV4::ErrorCode headerErrCode = header.read(stream);
    if (headerErrCode != PwHeaderV4::SUCCESS) {
        qDebug() << PwHeaderV4::getErrorMessage(headerErrCode) << headerErrCode;
        emit dbLoadError(PwHeaderV4::getErrorMessage(headerErrCode), headerErrCode);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_HEADER_READ);

    /* Calculate the AES key */
    ErrorCodesV4::ErrorCode err = transformKey(header, combinedKey, aesKey, UNLOCK_PROGRESS_HEADER_READ, UNLOCK_PROGRESS_KEY_TRANSFORMED);
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - transformKey" << err;
        emit dbLoadError(tr("Cannot decrypt database", "A generic error message"), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_KEY_TRANSFORMED);

    /* Decrypt data */
    int dataSize = dbBytes.size() - header.sizeInBytes();
    QByteArray decryptedData (dataSize, 0);
    // DB header not needed for decryption
    QByteArray dbBytesWithoutHeader = dbBytes.right(dataSize);
    err = decryptData(dbBytesWithoutHeader, decryptedData);
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - decryptData" << err;
        emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        return false;
    }
    emit progressChanged(UNLOCK_PROGRESS_DECRYPTED);

    QDataStream decryptedStream(decryptedData);
    decryptedStream.setByteOrder(QDataStream::LittleEndian);

    /* Verify first bytes */
    const int N = SB_SHA256_DIGEST_LEN;
    QByteArray startBytes(N, 0);
    decryptedStream.readRawData(startBytes.data(), N);
    if (startBytes != header.getStreamStartBytes()) {
        qDebug() << "First bytes do not match" << err;
        emit invalidPasswordOrKey();
        return false;
    }

    /* Read data blocks */
    QByteArray blocksData;
    err = readBlocks(decryptedStream, blocksData);
    Util::safeClear(decryptedData); // not needed any further
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - readBlocks" << err;
        emit dbLoadError(tr("Error reading database", "An error message"), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_BLOCKS_READ);

    QByteArray xmlData;
    if (header.isCompressed()) {
        /* Inflate GZip data to XML */
        Util::ErrorCode inflateErr = Util::inflateGZipData(blocksData, xmlData);
        Util::safeClear(blocksData);
        if (inflateErr != Util::SUCCESS) {
            qDebug() << "Error inflating database";
            emit dbLoadError(tr("Error inflating database", "An error message. Inflating means decompression of compressed data."), inflateErr);
            return false;
        }
    } else {
        xmlData = blocksData;
    }

    emit progressChanged(UNLOCK_PROGRESS_UNPACKED);

    /* Init Salsa20 for reading protected values */
    err = initSalsa20();
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Cannot decrypt database - initSalsa20" << err;
        emit dbLoadError(tr("Cannot decrypt database", "An error message"), err);
        return false;
    }

    /* Parse XML */
    QString xmlString = QString::fromUtf8(xmlData.data(), xmlData.size());
    err = parseXml(xmlString);
    Util::safeClear(xmlData);
    Util::safeClear(xmlString);
    if (err != ErrorCodesV4::SUCCESS) {
        qDebug() << "Error parsing database" << err;
        emit dbLoadError(tr("Cannot parse database", "An error message. Parsing refers to the analysis/understanding of file content (do not confuse with reading it)."), err);
        return false;
    }

    emit progressChanged(UNLOCK_PROGRESS_DONE);
    qDebug() << "DB unlocked";

    return true;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::initSalsa20() {
    CryptoManager* cm = CryptoManager::instance();

    QByteArray salsaKey;
    int err = cm->sha256(header.getProtectedStreamKey(), salsaKey);
    if (err != SB_SUCCESS)
        return ErrorCodesV4::CANNOT_INIT_SALSA20;

    salsa20.init(salsaKey, SALSA_20_INIT_VECTOR);
    return ErrorCodesV4::SUCCESS;
}

/**
 * Decrypts the DB's data using current keys.
 */
ErrorCodesV4::ErrorCode PwDatabaseV4::decryptData(const QByteArray& encryptedData, QByteArray& decryptedData) {
    // assert encryptedData.size is multiple of 16 bytes

    CryptoManager* cm = CryptoManager::instance();
    int err = cm->decryptAES(aesKey, header.getInitialVector(), encryptedData, decryptedData);
    if (err != SB_SUCCESS) {
        qDebug() << "decryptAES error: " << err;
        return ErrorCodesV4::CANNOT_DECRYPT_DB;
    }
    return ErrorCodesV4::SUCCESS;
}

/**
 * Extracts data blocks from the decrypted data stream, verifying hashes.
 */
ErrorCodesV4::ErrorCode PwDatabaseV4::readBlocks(QDataStream& inputStream, QByteArray& blocksData) {
    quint32 blockSize, readBlockId;
    QByteArray blockHash(SB_SHA256_DIGEST_LEN, 0);
    QByteArray computedHash(SB_SHA256_DIGEST_LEN, 0);

    Util::safeClear(blocksData);
    CryptoManager* cm = CryptoManager::instance();

    quint32 blockId = 0;
    while (true) {
        inputStream >> readBlockId;
        if (readBlockId != blockId) {
            qDebug() << "readBlocks wrong block ID";
            return ErrorCodesV4::WRONG_BLOCK_ID;
        }
        blockId++;

        inputStream.readRawData(blockHash.data(), SB_SHA256_DIGEST_LEN);
        inputStream >> blockSize;
        if (blockSize == 0) {
            if (Util::isAllZero(blockHash)) {
                break;
            } else {
                qDebug() << "readBlocks block hash is not all-zeros";
                return ErrorCodesV4::BLOCK_HASH_NON_ZERO;
            }
        }
        QByteArray blockData(blockSize, 0);
        inputStream.readRawData(blockData.data(), blockSize);
        int err = cm->sha256(blockData, computedHash);
        if ((err != SB_SUCCESS) || (computedHash != blockHash)) {
            qDebug() << "readBlocks block hash mismatch";
            return ErrorCodesV4::BLOCK_HASH_MISMATCH;
        }
        blocksData.append(blockData);
        Util::safeClear(blockData);
    }
    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::parseXml(const QString& xmlString) {
    if (_rootGroup) {
        delete _rootGroup;
        _rootGroup = NULL;
    }

    PwGroupV4* rootV4 = new PwGroupV4(this);
    rootV4->setDatabase(this);
    rootV4->setParentGroup(NULL); // not Qt parent, but the group containing this one

    ErrorCodesV4::ErrorCode err;
    QXmlStreamReader xml(xmlString);
    QStringRef tagName;
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNextStartElement()) {
            tagName = xml.name();
            if (tagName == XML_META) {
                err = meta.readFromStream(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (tagName == XML_ROOT) {
                if (xml.readNextStartElement() && (xml.name() == XML_GROUP)) {
                    err = loadGroupFromXml(xml, *rootV4);
                    if (err != ErrorCodesV4::SUCCESS)
                        return err;
                } else {
                    qDebug() << "SEVERE: there is no group in the root";
                    return ErrorCodesV4::XML_NO_ROOT_GROUP;
                }
            }
        }
    }
    xml.clear();
    if (xml.hasError())
        return ErrorCodesV4::XML_PARSING_ERROR;

    _rootGroup = rootV4;
//    debugPrint(_rootGroup, 2);
    return ErrorCodesV4::SUCCESS;
}

// loads the group and its children
ErrorCodesV4::ErrorCode PwDatabaseV4::loadGroupFromXml(QXmlStreamReader& xml, PwGroupV4& group) {
    Q_ASSERT(xml.name() == XML_GROUP);

    ErrorCodesV4::ErrorCode err;
    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_GROUP == tagName))) {
        if (xml.isStartElement()) {
            if (XML_UUID == tagName) {
                PwUuid uuid = PwStreamUtilsV4::readUuid(xml);
                group.setUuid(uuid);
                if (uuid == meta.getRecycleBinGroupUuid()) {
                    group.setDeleted(true); // may also be set higher in call stack
                }
            } else if (XML_ICON_ID == tagName) {
                QString iconIdStr = xml.readElementText();
                group.setIconId(iconIdStr.toInt(NULL));
            } else if (XML_NAME == tagName) {
                QString groupName = xml.readElementText();
                group.setName(groupName);
            } else if (XML_NOTES == tagName) {
                QString notes = xml.readElementText();
                group.setNotes(notes);
            } else if (XML_TIMES == tagName) {
                err = readGroupTimes(xml, group);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else if (XML_GROUP == tagName) {
                PwGroupV4* subGroup = new PwGroupV4(&group);
                subGroup->setDatabase(this);
                err = loadGroupFromXml(xml, *subGroup);
                if (err != ErrorCodesV4::SUCCESS) {
                    delete subGroup;
                    return err;
                }
                if (group.isDeleted())
                    subGroup->setDeleted(true); // propagate the deleted flag recursively
                group.addSubGroup(subGroup);
            } else if (XML_ENTRY == tagName) {
                PwEntryV4* entry = new PwEntryV4(&group);
                err = loadEntryFromXml(xml, *entry);
                if (err != ErrorCodesV4::SUCCESS) {
                    delete entry;
                    return err;
                }
                entry->setDeleted(group.isDeleted()); // propagate the deleted flag recursively
                group.addEntry(entry);
            }
        }
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError())
        return ErrorCodesV4::XML_PARSING_ERROR;
    else
        return ErrorCodesV4::SUCCESS;
}

// loads the entry
ErrorCodesV4::ErrorCode PwDatabaseV4::loadEntryFromXml(QXmlStreamReader& xml, PwEntryV4& entry) {
    Q_ASSERT(xml.name() == XML_ENTRY);

    entry.clear();
    bool isConversionOk;
    ErrorCodesV4::ErrorCode err = ErrorCodesV4::SUCCESS;

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_ENTRY == tagName))) {
        if (xml.isStartElement()) {
            if (XML_UUID == tagName) {
                PwUuid uuid = PwStreamUtilsV4::readUuid(xml);
                entry.setUuid(uuid);
            } else if (XML_ICON_ID == tagName) {
                int iconId = xml.readElementText().toInt(&isConversionOk, 10);
                if (isConversionOk)
                    entry.setIconId(iconId);
                else
                    return ErrorCodesV4::ICON_ID_IS_NOT_INTEGER;
            } else if (XML_STRING == tagName) {
                err = readEntryString(xml, entry);
            } else if (XML_BINARY == tagName) {
                PwAttachment* attachment = new PwAttachment(&entry);
                err = readEntryAttachment(xml, *attachment);
                entry.addAttachment(attachment);
            } else if (XML_TIMES == tagName) {
                err = readEntryTimes(xml, entry);
            } else if (XML_HISTORY == tagName) {
                err = readEntryHistory(xml, entry);
            }
        }
        if (err != ErrorCodesV4::SUCCESS)
            return err;
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_PARSING_ERROR;
    else
        return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::readGroupTimes(QXmlStreamReader& xml, PwGroupV4& group) {
    Q_ASSERT(XML_TIMES == xml.name());

    QString text;
    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_TIMES))) {
        if (xml.isStartElement()) {
            if (tagName == XML_LAST_MODIFICATION_TIME) {
                text = xml.readElementText();
                group.setLastModificationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_CREATION_TIME) {
                text = xml.readElementText();
                group.setCreationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_LAST_ACCESS_TIME) {
                text = xml.readElementText();
                group.setLastAccessTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_EXPIRY_TIME) {
                text = xml.readElementText();
                group.setExpiryTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_EXPIRES) {
                text = xml.readElementText();
                group.setExpires(text == XML_TRUE);
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_TIMES_PARSING_ERROR;
    else
        return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::readEntryTimes(QXmlStreamReader& xml, PwEntryV4& entry) {
    Q_ASSERT(XML_TIMES == xml.name());

    QString text;
    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_TIMES))) {
        if (xml.isStartElement()) {
            if (tagName == XML_LAST_MODIFICATION_TIME) {
                text = xml.readElementText();
                entry.setLastModificationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_CREATION_TIME) {
                text = xml.readElementText();
                entry.setCreationTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_LAST_ACCESS_TIME) {
                text = xml.readElementText();
                entry.setLastAccessTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_EXPIRY_TIME) {
                text = xml.readElementText();
                entry.setExpiryTime(QDateTime::fromString(text, Qt::ISODate));
            } else if (tagName == XML_EXPIRES) {
                text = xml.readElementText();
                entry.setExpires(text == XML_TRUE);
            }
        }
        xml.readNext();
        tagName = xml.name();
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_TIMES_PARSING_ERROR;
    else
        return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::readEntryHistory(QXmlStreamReader& xml, PwEntryV4& hostEntry) {
    Q_ASSERT(XML_HISTORY == xml.name());

    ErrorCodesV4::ErrorCode err;
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_HISTORY))) {
        if (xml.isStartElement() && (tagName == XML_ENTRY)) {
            PwEntryV4* historyEntry = new PwEntryV4(&hostEntry); // hostEntry is a parent, not a copy source
            err = loadEntryFromXml(xml, *historyEntry);
            if (err != ErrorCodesV4::SUCCESS) {
                delete historyEntry;
                return err;
            }
            hostEntry.addHistoryEntry(historyEntry);
        }
        xml.readNext();
        tagName = xml.name();
    }
    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::readEntryString(QXmlStreamReader& xml, PwEntryV4& entry) {
    Q_ASSERT(XML_STRING == xml.name());

    QString key, value;
    QStringRef tagName = xml.name();

    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_STRING))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                key = xml.readElementText();
            } else if (tagName == XML_VALUE) {
                ErrorCodesV4::ErrorCode err = readEntryStringValue(xml, value);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            }
        }
    }
    entry.setField(key, value);
    return ErrorCodesV4::SUCCESS;
}

// read a value from XML, decrypting it if necessary
ErrorCodesV4::ErrorCode PwDatabaseV4::readEntryStringValue(QXmlStreamReader& xml, QString& value) {
    Q_ASSERT(xml.name() == XML_VALUE);

    QXmlStreamAttributes attr = xml.attributes();

    if (attr.value(XML_PROTECTED) == XML_TRUE) {
        QByteArray valueBytes = PwStreamUtilsV4::readBase64(xml);
        int size = valueBytes.length();

        QByteArray salsaBytes;
        salsa20.getBytes(salsaBytes, size);

        const char* xorBuf = salsaBytes.constData();
        char* valueBuf = valueBytes.data();
        for (int i = 0; i < size; i++) {
            valueBuf[i] ^= xorBuf[i];
        }
        value = QString::fromUtf8(valueBuf, size);
    } else {
        value = xml.readElementText();
    }

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwDatabaseV4::readEntryAttachment(QXmlStreamReader& xml, PwAttachment& attachment) {
    Q_ASSERT(XML_BINARY == xml.name());

    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_BINARY))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                QString fileName = xml.readElementText();
                attachment.setName(fileName);
            } else if (tagName == XML_VALUE) {
                QString binaryRef = xml.attributes().value(XML_REF).toString();
                PwBinaryV4* binary = meta.getBinaryByReference(binaryRef);
                if (!binary) {
                    return ErrorCodesV4::INVALID_ATTACHMENT_REFERENCE;
                }
                attachment.setData(binary->getData(), binary->isCompressed());
            }
        }
    }
    return ErrorCodesV4::SUCCESS;
}

/**
 * Encrypts and writes DB content to the given array.
 */
bool PwDatabaseV4::save(QByteArray& outData) {
    //TODO implement V4 saving
    Q_UNUSED(outData); // remove when method is implemented
    return false;
}
