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


PwExtraField::PwExtraField(QObject* parent) : QObject(parent),
        _name(""), _value("") {
    // left empty
}

PwExtraField::PwExtraField(QObject* parent, const QString& name, const QString& value) : QObject(parent),
        _name(name), _value(value) {
    // left empty
}

PwExtraField::~PwExtraField() {
    Util::safeClear(_name);
    Util::safeClear(_value);
}

QString PwExtraField::toString() const {
    return _name + " = " + _value;
}

bool PwExtraField::matchesQuery(const QString& query) const {
    return getName().contains(query, Qt::CaseInsensitive) ||
            getValue().contains(query, Qt::CaseInsensitive);
}

/**************************/

PwEntryV4::PwEntryV4(QObject* parent) :
        PwEntry(parent),
        fields(),
        _extraFieldsDataModel(),
        _historyDataModel() {
    _extraFieldsDataModel.setParent(this);
    _historyDataModel.setParent(this);
}

PwEntryV4::~PwEntryV4() {
    clear();
}

void PwEntryV4::clear() {
    _historyDataModel.clear(); // deletes owned objects
    emit historySizeChanged(0);

    _extraFieldsDataModel.clear(); // deletes owned objects
    emit extraSizeChanged(0);

    fields.clear();
    PwEntry::clear();
}

bool PwEntryV4::isStandardField(const QString& name) const {
    return (name == TITLE) || (name == USERNAME) || (name == PASSWORD) || (name == URL) || (name == NOTES);
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

void PwEntryV4::setField(const QString& name, const QString& value) {
    fields.insert(name, value);
    if (!isStandardField(name)) {
        addExtraField(name, value);
    }
}

void PwEntryV4::addExtraField(const QString& name, const QString& value) {
    PwExtraField* ef = new PwExtraField(this, name, value);
    _extraFieldsDataModel.append(ef); // implicitly takes ownership
    emit extraSizeChanged(_extraFieldsDataModel.size());
}

void PwEntryV4::addHistoryEntry(PwEntryV4* historyEntry) {
    _historyDataModel.append(historyEntry); // implicitly takes ownership
    emit historySizeChanged(_historyDataModel.size());
}

bb::cascades::DataModel* PwEntryV4::getExtraFieldsDataModel() {
    return &_extraFieldsDataModel;
}

bb::cascades::DataModel* PwEntryV4::getHistoryDataModel() {
    return &_historyDataModel;
}

QString PwEntryV4::getTitle() const {
    return fields.value(TITLE);
}
void PwEntryV4::setTitle(const QString& title) {
    setField(TITLE, title);
}
QString PwEntryV4::getUserName() const {
    return fields.value(USERNAME);
}
void PwEntryV4::setUserName(const QString& userName) {
    setField(USERNAME, userName);
}
QString PwEntryV4::getPassword() const {
    return fields.value(PASSWORD);
}
void PwEntryV4::setPassword(const QString& password) {
    setField(PASSWORD, password);
}
QString PwEntryV4::getUrl() const {
    return fields.value(URL);
}
void PwEntryV4::setUrl(const QString& url) {
    setField(URL, url);
}
QString PwEntryV4::getNotes() const {
    return fields.value(NOTES);
}
void PwEntryV4::setNotes(const QString& notes) {
    setField(NOTES, notes);
}

/** Returns a new entry instance with the same field values. */
PwEntry* PwEntryV4::clone() {
    // TODO implement PwEntryV4::clone
    return NULL;
}

/**
 * Makes a backup copy of the current values/state of the entry.
 * (For V4 adds the current state to entry's history)
 * Returns true if successful.
 */
bool PwEntryV4::backupState() {
    // TODO implement PwEntryV4::backupState()
    return false;
}

/**
 * Loads the given file and attaches it to the entry.
 * Makes a backup of the initial entry state.
 * Returns true if successful, false in case of any error.
 */
bool PwEntryV4::attachFile(const QString& filePath) {
    // TODO implement PwEntryV4::attachFile
    Q_UNUSED(filePath);
    return false;
}

/**
 * Loads entry fields from the stream.
 * The caller is responsible for clearing any previous values.
 */
ErrorCodesV4::ErrorCode PwEntryV4::readFromStream(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20) {
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
            } else if (XML_STRING == tagName) {
                err = readString(xml, meta, salsa20);
            } else if (XML_BINARY == tagName) {
                PwAttachment* attachment = new PwAttachment(this);
                err = readAttachment(xml, meta, salsa20, *attachment);
                addAttachment(attachment);
            } else if (XML_TIMES == tagName) {
                err = readTimes(xml);
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

ErrorCodesV4::ErrorCode PwEntryV4::readHistory(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20) {
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
        return ErrorCodesV4::XML_ENTRY_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwEntryV4::readString(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20) {
    Q_ASSERT(XML_STRING == xml.name());

    QString key, value;
    QStringRef tagName = xml.name();

    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_STRING))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                key = PwStreamUtilsV4::readString(xml);
            } else if (tagName == XML_VALUE) {
                ErrorCodesV4::ErrorCode err = readStringValue(xml, meta, salsa20, value);
                if (err != ErrorCodesV4::SUCCESS)
                    return err;
            } else {
                qDebug() << "unknown tag in PwEntryV4::readString():" << tagName;
                return ErrorCodesV4::XML_ENTRY_PARSING_ERROR;
            }
        }
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_PARSING_ERROR;

    setField(key, value);
    return ErrorCodesV4::SUCCESS;
}

// read a value from XML, decrypting it if necessary
ErrorCodesV4::ErrorCode PwEntryV4::readStringValue(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20, QString& value) {
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
        value = PwStreamUtilsV4::readString(xml);
    }

    return ErrorCodesV4::SUCCESS;
}

ErrorCodesV4::ErrorCode PwEntryV4::readAttachment(QXmlStreamReader &xml, PwMetaV4& meta, Salsa20& salsa20, PwAttachment &attachment) {
    Q_ASSERT(XML_BINARY == xml.name());

    QStringRef tagName = xml.name();
    while (!xml.hasError() && !(xml.isEndElement() && (tagName == XML_BINARY))) {
        xml.readNext();
        tagName = xml.name();
        if (xml.isStartElement()) {
            if (tagName == XML_KEY) {
                attachment.setName(PwStreamUtilsV4::readString(xml));
            } else if (tagName == XML_VALUE) {
                QString binaryRef = xml.attributes().value(XML_REF).toString();
                //parent group is not defined here yet

                PwBinaryV4* binary = meta.getBinaryByReference(binaryRef);
                if (!binary) {
                    return ErrorCodesV4::INVALID_ATTACHMENT_REFERENCE;
                }
                attachment.setData(binary->getData(), binary->isCompressed());
            }
        }
    }
    if (xml.hasError())
        return ErrorCodesV4::XML_ENTRY_PARSING_ERROR;

    return ErrorCodesV4::SUCCESS;
}
