/*
 * PwEntryV4.cpp
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#include "db/v4/PwEntryV4.h"
#include "util/Util.h"
#include "db/v4/PwMetaV4.h"
#include "db/v4/PwStreamUtilsV4.h"

// Standard entry fields
const static QString TITLE = QString("Title");
const static QString USERNAME = QString("UserName");
const static QString PASSWORD = QString("Password");
const static QString URL = QString("URL");
const static QString NOTES = QString("Notes");


PwField::PwField(QObject* parent) : QObject(parent),
        _name(""), _value(""), _isProtected(false) {
    // left empty
}

PwField::PwField(QObject* parent, const QString& name, const QString& value, const bool isProtected) : QObject(parent),
        _name(name), _value(value), _isProtected(isProtected) {
    // left empty
}

PwField::~PwField() {
    clear();
}

void PwField::clear() {
    Util::safeClear(_name);
    Util::safeClear(_value);
    _isProtected = false;
}

PwField* PwField::clone() const {
    PwField* copy = new PwField(this->parent(), _name, _value, _isProtected);
    return copy;
}

QString PwField::toString() const {
    return (_isProtected ? "<protected> " : "<not protected> ") + _name + " = " + _value;
}

bool PwField::matchesQuery(const QString& query) const {
    return getName().contains(query, Qt::CaseInsensitive) ||
            getValue().contains(query, Qt::CaseInsensitive);
}

void PwField::setName(const QString& name) {
    if (_name != name) {
        _name = Util::deepCopy(name);
        emit nameChanged(_name);
    }
}

void PwField::setValue(const QString& value) {
    if (_value != value) {
        _value = Util::deepCopy(value);
        emit valueChanged(_value);
    }
}
void PwField::setProtected(const bool isProtected) {
    if (_isProtected != isProtected) {
        _isProtected = isProtected;
        emit protectedChanged(isProtected);
    }
}

bool PwField::isStandardField() const {
    return (_name == TITLE) || (_name == USERNAME) || (_name == PASSWORD) || (_name == URL) || (_name == NOTES);
}

/**
 * Sets field's in memory protection flag to that specified in Meta's properties.
 * Only applies for standard fields, does nothing for the others.
 */
void PwField::updateProtectionFlag(const PwMetaV4& meta) {
    const MemoryProtection* mp = meta.getMemoryProtection();
    if (_name == TITLE) {
        setProtected(mp->isProtectTitle());
    } else if (_name == USERNAME) {
        setProtected(mp->isProtectUserName());
    } else if (_name == PASSWORD) {
        setProtected(mp->isProtectPassword());
    } else if (_name == URL) {
        setProtected(mp->isProtectUrl());
    } else if (_name == NOTES) {
        setProtected(mp->isProtectNotes());
    }
}

ErrorCodesV4::ErrorCode PwField::readFromStream(QXmlStreamReader& xml, Salsa20& salsa20) {
    Q_ASSERT(XML_STRING == xml.name());

    QString key, value;
    bool protect = false;

    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_STRING))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                key = PwStreamUtilsV4::readString(xml);
            } else if (tagName == XML_VALUE) {
                if (xml.attributes().value(XML_PROTECTED) == XML_TRUE) {
                    // the value is encrypted, need to decrypt it first
                    protect = true;
                    QByteArray valueBytes = PwStreamUtilsV4::readBase64(xml);
                    salsa20.xorWithNextBytes(valueBytes);
                    value = QString::fromUtf8(valueBytes, valueBytes.length());
                } else {
                    // simple plain-text value
                    protect = false;
                    value = PwStreamUtilsV4::readString(xml);
                }
            } else {
                qDebug() << "unknown tag in PwField::readFromStream():" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_ENTRY_FIELD_PARSING_ERROR;
            }
        }
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_FIELD_PARSING_ERROR;

    // Even if either XML_KEY or XML_VALUE were missing in the stream, (re)init the field anyway
    setName(key);
    setValue(value);
    setProtected(protect);

    return ErrorCodesV4::SUCCESS;
}

void PwField::writeToStream(QXmlStreamWriter& xml, Salsa20& salsa20) const {
    xml.writeStartElement(XML_STRING);
    PwStreamUtilsV4::writeString(xml, XML_KEY, getName());
    if (isProtected()) {
        xml.writeStartElement(XML_VALUE);
        xml.writeAttribute(XML_PROTECTED, XML_TRUE);

        QByteArray encValue = getValue().toUtf8();
        salsa20.xorWithNextBytes(encValue);
        xml.writeCharacters(encValue.toBase64());
        xml.writeEndElement(); // XML_VALUE
    } else {
        PwStreamUtilsV4::writeString(xml, XML_VALUE, getValue());
    }
    xml.writeEndElement(); // XML_STRING
}

/**************************/
PwAutoType::PwAutoType() {
    clear();
}

PwAutoType::PwAutoType(const PwAutoType& original) :
    _enabled(original._enabled),
    _obfuscationType(original._obfuscationType),
    _defaultSequence(Util::deepCopy(original._defaultSequence)), // this is subject to safeClear(), so need a deep copy
    _associations(original._associations) { // a shallow copy, since we don't safeClear this list

}

PwAutoType::~PwAutoType() {
    clear();
}

void PwAutoType::clear() {
    _enabled = true;
    _obfuscationType = 0;
    Util::safeClear(_defaultSequence);
    _associations.clear();
}

ErrorCodesV4::ErrorCode PwAutoType::readFromStream(QXmlStreamReader& xml) {
    Q_ASSERT(XML_AUTO_TYPE == xml.name());

    QString key, value;
    ErrorCodesV4::ErrorCode err;

    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_AUTO_TYPE))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_AUTO_TYPE_ENABLED) {
                _enabled = PwStreamUtilsV4::readBool(xml, true);
            } else if (tagName == XML_AUTO_TYPE_OBFUSCATION) {
                _obfuscationType = PwStreamUtilsV4::readUInt32(xml, 0);
            } else if (tagName == XML_AUTO_TYPE_DEFAULT_SEQUENCE) {
                _defaultSequence = PwStreamUtilsV4::readString(xml);
            } else if (tagName == XML_AUTO_TYPE_ITEM) {
                err = readAssociation(xml);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else {
                qDebug() << "unknown tag in PwAutoType:" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_ENTRY_AUTO_TYPE_PARSING_ERROR;
            }
        }
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_AUTO_TYPE_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwAutoType::readAssociation(QXmlStreamReader& xml) {
    Q_ASSERT(XML_AUTO_TYPE_ITEM == xml.name());

    QString window, sequence;
    QStringRef tagName = xml.name();

    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_AUTO_TYPE_ITEM))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_AUTO_TYPE_WINDOW) {
                window = PwStreamUtilsV4::readString(xml);
            } else if (tagName == XML_AUTO_TYPE_KEYSTROKE_SEQUENCE) {
                sequence = PwStreamUtilsV4::readString(xml);
            } else {
                qDebug() << "unknown tag in PwAutoType/association:" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_ENTRY_AUTO_TYPE_ASSOCIATION_PARSING_ERROR;
            }
        }
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_AUTO_TYPE_ASSOCIATION_PARSING_ERROR;

    _associations.append(QStringPair(window, sequence));
    return ErrorCodesV4::SUCCESS;
}

void PwAutoType::writeToStream(QXmlStreamWriter& xml) const {
    xml.writeStartElement(XML_AUTO_TYPE);
    PwStreamUtilsV4::writeBool(xml, XML_AUTO_TYPE_ENABLED, _enabled);
    PwStreamUtilsV4::writeUInt32(xml, XML_AUTO_TYPE_OBFUSCATION, _obfuscationType);
    if (!_defaultSequence.isEmpty())
        PwStreamUtilsV4::writeString(xml, XML_AUTO_TYPE_DEFAULT_SEQUENCE, _defaultSequence);

    for (int i = 0; i < _associations.size(); i++) {
        QPair<QString, QString> item = _associations.at(i);
        xml.writeStartElement(XML_AUTO_TYPE_ITEM);
        PwStreamUtilsV4::writeString(xml, XML_AUTO_TYPE_WINDOW, item.first);
        PwStreamUtilsV4::writeString(xml, XML_AUTO_TYPE_KEYSTROKE_SEQUENCE, item.second);
        xml.writeEndElement(); // XML_AUTO_TYPE_ITEM
    }
    xml.writeEndElement(); // XML_AUTO_TYPE
}


/**************************/

PwEntryV4::PwEntryV4(QObject* parent) : PwEntry(parent) {
    // without setParent() the app crashes with repeated freeing of free memory
    _extraFieldsDataModel.setParent(this);
    _historyDataModel.setParent(this);

    clear();
}

PwEntryV4::~PwEntryV4() {
    clear();
}

void PwEntryV4::clear() {
    _customIconUuid.clear();
    _autoType.clear();
    setUsageCount(0);
    setLocationChangedTime(QDateTime::currentDateTime());
    Util::safeClear(_foregroundColor);
    Util::safeClear(_backgroundColor);
    Util::safeClear(_overrideUrl);
    Util::safeClear(_tags);

    _historyDataModel.clear(); // deletes owned objects
    emit historySizeChanged(0);

    _extraFieldsDataModel.clear(); // deletes owned objects
    emit extraSizeChanged(0);

    fields.clear();
    PwEntry::clear();
}

/** Updates last access timestamp to current time and increases usage counter */
void PwEntryV4::registerAccessEvent() {
    PwEntry::registerAccessEvent();
    setUsageCount(getUsageCount() + 1);
}

/** Search helper. Returns true if any of the fields contain the query string. */
bool PwEntryV4::matchesQuery(const QString& query) const {
    if (PwEntry::matchesQuery(query))
        return true;

    for (int i = 0; i < _extraFieldsDataModel.size(); i++) {
        if (_extraFieldsDataModel.value(i)->matchesQuery(query))
            return true;
    }
    return false;
}

void PwEntryV4::addField(PwField* field) {
    fields.insert(field->getName(), field);
    if (!field->isStandardField()) {
        // both fields and _extraFieldsDataModel contain pointer to the same PwField instance
        _extraFieldsDataModel.append(field); // takes ownership, if not already set
        emit extraSizeChanged(_extraFieldsDataModel.size());
    }
}

void PwEntryV4::setField(const QString& name, const QString& value) {
    PwField* field = fields.value(name);
    if (field != NULL) {
        field->setValue(value);
        // Memory protection flag remains unchanged.
    } else {
        // Set memory protection off by default:
        // - for standard fields it will be updated according to DB Meta protection flags (on save)
        // - for other fields, it will be changed by some other method.
        field = new PwField(this, name, value, false);
        addField(field);
    }
}

void PwEntryV4::addHistoryEntry(PwEntryV4* historyEntry) {
    _historyDataModel.append(historyEntry); // implicitly takes ownership
    emit historySizeChanged(_historyDataModel.size());
}

void PwEntryV4::clearHistory() {
    _historyDataModel.clear();
    emit historySizeChanged(_historyDataModel.size());
}

bb::cascades::DataModel* PwEntryV4::getExtraFieldsDataModel() {
    return &_extraFieldsDataModel;
}

bb::cascades::DataModel* PwEntryV4::getHistoryDataModel() {
    return &_historyDataModel;
}

QString PwEntryV4::getTitle() const {
    PwField* field = fields.value(TITLE);
    return (field == NULL) ? "" : field->getValue();
}
void PwEntryV4::setTitle(const QString& title) {
    if (title != getTitle()) {
        setField(TITLE, title);
        emit titleChanged(title);
    }
}
QString PwEntryV4::getUserName() const {
    PwField* field = fields.value(USERNAME);
    return (field == NULL) ? "" : field->getValue();
}
void PwEntryV4::setUserName(const QString& userName) {
    if (userName != getUserName()) {
        setField(USERNAME, userName);
        emit userNameChanged(userName);
    }
}
QString PwEntryV4::getPassword() const {
    PwField* field = fields.value(PASSWORD);
    return (field == NULL) ? "" : field->getValue();
}
void PwEntryV4::setPassword(const QString& password) {
    if (password != getPassword()) {
        setField(PASSWORD, password);
        emit passwordChanged(password);
    }
}
QString PwEntryV4::getUrl() const {
    PwField* field = fields.value(URL);
    return (field == NULL) ? "" : field->getValue();
}
void PwEntryV4::setUrl(const QString& url) {
    if (url != getUrl()) {
        setField(URL, url);
        emit urlChanged(url);
    }
}
QString PwEntryV4::getNotes() const {
    PwField* field = fields.value(NOTES);
    return (field == NULL) ? "" : field->getValue();
}
void PwEntryV4::setNotes(const QString& notes) {
    if (notes != getNotes()) {
        setField(NOTES, notes);
        emit notesChanged(notes);
    }
}

void PwEntryV4::setCustomIconUuid(const PwUuid& uuid) {
    if (_customIconUuid != uuid) {
        _customIconUuid = uuid;
        emit customIconUuidChanged(_customIconUuid);
    }
}

void PwEntryV4::setUsageCount(const quint32 usageCount) {
    if (usageCount != _usageCount) {
        _usageCount = usageCount;
        emit usageCountChanged(usageCount);
    }
}
void PwEntryV4::setLocationChangedTime(const QDateTime& locationChangedTime) {
    if (locationChangedTime != _locationChangedTime) {
        _locationChangedTime = locationChangedTime;
        emit locationChangedTimeChanged(locationChangedTime);
    }
}
void PwEntryV4::setForegroundColor(const QString& fgColor) {
    if (fgColor != _foregroundColor) {
        _foregroundColor = Util::deepCopy(fgColor);
        emit foregroundColorChanged(_foregroundColor);
    }
}
void PwEntryV4::setBackgroundColor(const QString& bgColor) {
    if (bgColor != _backgroundColor) {
        _backgroundColor = Util::deepCopy(bgColor);
        emit backgroundColorChanged(_backgroundColor);
    }
}
void PwEntryV4::setOverrideUrl(const QString& url) {
    if (url != _overrideUrl) {
        _overrideUrl = Util::deepCopy(url);
        emit overrideUrlChanged(_overrideUrl);
    }
}
void PwEntryV4::setTags(const QString& tags) {
    if (tags != _tags) {
        _tags = Util::deepCopy(tags);
        emit tagsChanged(_tags);
    }
}


/** Returns a new entry instance with the same field values. */
PwEntry* PwEntryV4::clone() {
    PwEntryV4* entryCopy = new PwEntryV4(this->parent());

    entryCopy->setUuid(getUuid());
    entryCopy->setIconId(getIconId());
    entryCopy->setCustomIconUuid(getCustomIconUuid());
    entryCopy->setForegroundColor(getForegroundColor());
    entryCopy->setBackgroundColor(getBackgroundColor());
    entryCopy->setOverrideUrl(getOverrideUrl());
    entryCopy->setTags(getTags());

    QMapIterator<QString, PwField*> iter(fields);
    while (iter.hasNext()) {
        iter.next();
        PwField* fieldCopy = iter.value()->clone();
        fieldCopy->setParent(NULL); // will be taken by the container
        entryCopy->addField(fieldCopy);
    }

    PwAttachmentDataModel* attachDataModel = getAttachmentsDataModel();
    for (int i = 0; i < attachDataModel->size(); i++) {
        PwAttachment* attCopy = attachDataModel->value(i)->clone();
        attCopy->setParent(NULL); // will be taken by the data model
        entryCopy->addAttachment(attCopy);
    }

    entryCopy->_autoType = _autoType;

    entryCopy->setLastModificationTime(getLastModificationTime());
    entryCopy->setCreationTime(getCreationTime());
    entryCopy->setLastAccessTime(getLastAccessTime());
    entryCopy->setExpiryTime(getExpiryTime());
    entryCopy->setExpires(isExpires());
    entryCopy->setUsageCount(getUsageCount());
    entryCopy->setLocationChangedTime(getLocationChangedTime());

    for (int i = 0; i < _historyDataModel.size(); i++) {
        PwEntryV4* historyCopy = dynamic_cast<PwEntryV4*>(_historyDataModel.value(i)->clone());
        historyCopy->setParent(NULL); // will be taken by history data model
        entryCopy->addHistoryEntry(historyCopy);
    }

    // The clone is not inserted into the parent group because
    // sometimes the clones are needed in entry history (i.e. no parent group)
    // getParentGroup()->addEntry(copy);

    return entryCopy;
}

/**
 * Makes a backup copy of the current values/state of the entry.
 * Returns true if successful.
 */
bool PwEntryV4::backupState() {
    // KeePass 2 backups entry state by adding its copy to history.

    // Also, in V4 historical items preserve the same UUID (unlike in V3)
    PwEntryV4* entryCopy = dynamic_cast<PwEntryV4*>(this->clone());
    entryCopy->clearHistory(); // historical entries must not have history
    addHistoryEntry(entryCopy);

    return true;
}

/**
 * Loads the given file and attaches it to the entry.
 * Makes a backup of the initial entry state.
 * Returns true if successful, false in case of any error.
 */
bool PwEntryV4::attachFile(const QString& filePath) {
    PwAttachment* newAtt = PwAttachment::createFromFile(filePath);
    if (!newAtt)
        return false;

    registerModificationEvent();
    backupState();

    return addAttachment(newAtt);
}

/**
 * Loads entry fields from the stream.
 * The caller is responsible for clearing any previous values.
 */
ErrorCodesV4::ErrorCode PwEntryV4::readFromStream(QXmlStreamReader& xml, const PwMetaV4& meta, Salsa20& salsa20) {
    Q_ASSERT(xml.name() == XML_ENTRY);

    ErrorCodesV4::ErrorCode err = ErrorCodesV4::SUCCESS;

    xml.readNext();
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (XML_ENTRY == tagName))) {
        if (xml.isStartElement()) {
            if (XML_UUID == tagName) {
                setUuid(PwStreamUtilsV4::readUuid(xml));
            } else if (XML_ICON_ID == tagName) {
                setIconId(PwStreamUtilsV4::readInt32(xml, 0));
            } else if (XML_CUSTOM_ICON_UUID == tagName) {
                setCustomIconUuid(PwStreamUtilsV4::readUuid(xml));
            } else if (XML_FOREGROUND_COLOR == tagName) {
                setForegroundColor(PwStreamUtilsV4::readString(xml));
            } else if (XML_BACKGROUND_COLOR == tagName) {
                setBackgroundColor(PwStreamUtilsV4::readString(xml));
            } else if (XML_OVERRIDE_URL == tagName) {
                setOverrideUrl(PwStreamUtilsV4::readString(xml));
            } else if (XML_TAGS == tagName) {
                setTags(PwStreamUtilsV4::readString(xml));
            } else if (XML_STRING == tagName) {
                PwField* field = new PwField(this);
                err = field->readFromStream(xml, salsa20);
                addField(field);
            } else if (XML_BINARY == tagName) {
                PwAttachment* attachment = new PwAttachment(this);
                err = readAttachment(xml, meta, salsa20, *attachment);
                addAttachment(attachment);
            } else if (XML_TIMES == tagName) {
                err = readTimes(xml);
            } else if (XML_AUTO_TYPE == tagName) {
                err = _autoType.readFromStream(xml);
            } else if (XML_HISTORY == tagName) {
                err = readHistory(xml, meta, salsa20);
            }
        }
        if (err != ErrorCodesV4::SUCCESS)
            return err;
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwEntryV4::readTimes(QXmlStreamReader& xml) {
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
                qDebug() << "unknown PwEntryV4/Times tag:" << tagName;
                PwStreamUtilsV4::readUnknown(xml);
                return ErrorCodesV4::XML_ENTRY_TIMES_PARSING_ERROR;
            }
        }
        if (!conversionOk)
            break;

        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError() || !conversionOk)
        return ErrorCodesV4::XML_ENTRY_TIMES_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwEntryV4::readHistory(QXmlStreamReader& xml, const PwMetaV4& meta, Salsa20& salsa20) {
    Q_ASSERT(XML_HISTORY == xml.name());

    ErrorCodesV4::ErrorCode err;
    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_HISTORY))) {
        if (xml.isStartElement() && (tagName == XML_ENTRY)) {
            PwEntryV4* historyEntry = new PwEntryV4(this); // 'this' is the parent, not a copy source
            err = historyEntry->readFromStream(xml, meta, salsa20);
            if (err != ErrorCodesV4::SUCCESS)
                return err;

            this->addHistoryEntry(historyEntry);
        }
        xml.readNext();
        tagName = xml.name();
    }

    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_HISTORY_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwEntryV4::readAttachment(QXmlStreamReader &xml, const PwMetaV4& meta, Salsa20& salsa20, PwAttachment& attachment) {
    Q_ASSERT(XML_BINARY == xml.name());

    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_BINARY))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                attachment.setName(PwStreamUtilsV4::readString(xml));
            } else if (tagName == XML_VALUE) {
                QString binaryIdStr = xml.attributes().value(XML_REF).toString();
                bool convOk;
                int binaryId = binaryIdStr.toInt(&convOk);
                if (convOk) {
                    //parent group is not defined here yet
                    PwBinaryV4* binary = meta.getBinaryById(binaryId);
                    if (!binary) {
                        return ErrorCodesV4::INVALID_ATTACHMENT_REFERENCE;
                    }
                    attachment.setData(binary->getData(), binary->isCompressed());
                    attachment.setId(binaryId);
                } else {
                    return ErrorCodesV4::INVALID_ATTACHMENT_REFERENCE;
                }
            }
        }
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_ATTACHMENT_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

/**
 * Writes the entry to the stream.
 */
void PwEntryV4::writeToStream(QXmlStreamWriter& xml, PwMetaV4& meta, Salsa20& salsa20) {
    qDebug() << "writing entry UUID" << getUuid().toString();
    xml.writeStartElement(XML_ENTRY);
    PwStreamUtilsV4::writeUuid(xml, XML_UUID, getUuid());
    PwStreamUtilsV4::writeInt32(xml, XML_ICON_ID, getIconId());
    PwUuid customIconUuid = getCustomIconUuid();
    if (!customIconUuid.isEmpty())
        PwStreamUtilsV4::writeUuid(xml, XML_CUSTOM_ICON_UUID, customIconUuid);
    PwStreamUtilsV4::writeString(xml, XML_FOREGROUND_COLOR, getForegroundColor());
    PwStreamUtilsV4::writeString(xml, XML_BACKGROUND_COLOR, getBackgroundColor());
    PwStreamUtilsV4::writeString(xml, XML_OVERRIDE_URL, getOverrideUrl());
    PwStreamUtilsV4::writeString(xml, XML_TAGS, getTags());

    // write times
    xml.writeStartElement(XML_TIMES);
    PwStreamUtilsV4::writeTime(xml, XML_LAST_MODIFICATION_TIME, getLastModificationTime());
    PwStreamUtilsV4::writeTime(xml, XML_CREATION_TIME, getCreationTime());
    PwStreamUtilsV4::writeTime(xml, XML_LAST_ACCESS_TIME, getLastAccessTime());
    PwStreamUtilsV4::writeTime(xml, XML_EXPIRY_TIME, getExpiryTime());
    PwStreamUtilsV4::writeBool(xml, XML_EXPIRES, isExpires());
    PwStreamUtilsV4::writeUInt32(xml, XML_USAGE_COUNT, getUsageCount());
    PwStreamUtilsV4::writeTime(xml, XML_LOCATION_CHANGED_TIME, getLocationChangedTime());
    xml.writeEndElement(); // XML_TIMES

    // write <String> fields
    QMapIterator<QString, PwField*> iter(fields);
    while (iter.hasNext()) {
        iter.next();
        QString key = iter.key();
        PwField* field = iter.value();
        field->updateProtectionFlag(meta);
        field->writeToStream(xml, salsa20);
    }

    writeAttachments(xml);

    _autoType.writeToStream(xml);

    int historySize = _historyDataModel.size();
    if (historySize > 0) {
        xml.writeStartElement(XML_HISTORY);
        for (int i = 0; i < historySize; i++) {
            PwEntryV4* historyEntry = _historyDataModel.value(i);
            historyEntry->writeToStream(xml, meta, salsa20);
        }
        xml.writeEndElement(); // XML_HISTORY
    }

    xml.writeEndElement(); // XML_ENTRY
}

void PwEntryV4::writeAttachments(QXmlStreamWriter& xml) {
    PwAttachmentDataModel* attachDataModel = getAttachmentsDataModel();
    for (int i = 0; i < attachDataModel->size(); i++) {
        PwAttachment* att = attachDataModel->value(i);
        xml.writeStartElement(XML_BINARY);

        PwStreamUtilsV4::writeString(xml, XML_KEY, att->getName());

        xml.writeEmptyElement(XML_VALUE);
        xml.writeAttribute(XML_REF, QString::number(att->getId()));

        xml.writeEndElement(); // XML_BINARY
    }
}
