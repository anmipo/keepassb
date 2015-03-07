/*
 * PwEntryV4.h
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#ifndef PWENTRYV4_H_
#define PWENTRYV4_H_

#include "db/PwEntry.h"
#include "db/v4/DefsV4.h"
#include "db/v4/PwMetaV4.h"
#include "crypto/CryptoManager.h"
#include <QMetaType>
#include <QList>
#include <bb/cascades/DataModel>
#include <bb/cascades/QListDataModel>
#include <QtXml/QXmlStreamReader>

/**
 * Field of a V4 database entry
 */
class PwField: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(QString value READ getValue NOTIFY valueChanged)
    Q_PROPERTY(QString protected READ isProtected NOTIFY protectedChanged)
private:
    QString _name;
    QString _value;
    bool _isProtected;

public:
    PwField(QObject* parent=0);
    PwField(QObject* parent, const QString& name, const QString& value, const bool isProtected);
    virtual ~PwField();

    /** True if field's name corresponds to one of the fixed/standard V4 fields. */
    bool isStandardField() const;

    void clear();

    PwField* clone() const;

    /**
     * Sets field's in memory protection flag to that specified in Meta's properties.
     * Only applies for standard fields, does nothing for the others.
     */
    void updateProtectionFlag(const PwMetaV4& meta);

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml, Salsa20& salsa20);
    void writeToStream(QXmlStreamWriter& xml, Salsa20& salsa20) const;

    /** Returns true if any string contains the query string. */
    virtual bool matchesQuery(const QString& query) const;

    Q_INVOKABLE QString toString() const;

    // property accessors
    QString getName() const { return _name; }
    QString getValue() const { return _value; }
    bool isProtected() const { return _isProtected; }
    void setName(const QString& name);
    void setValue(const QString& value);
    void setProtected(const bool isProtected);
signals:
    // nameChanged and valueChanged are never emitted
    void nameChanged(QString name);
    void valueChanged(QString value);
    void protectedChanged(bool prot);
};

/**
 * Auto type settings of a V4 entry
 */
class PwAutoType {
private:
    bool _enabled;
    quint32 _obfuscationType;
    QString _defaultSequence;
    QList<QStringPair> _associations;

    ErrorCodesV4::ErrorCode readAssociation(QXmlStreamReader& xml);
public:
    PwAutoType();
    PwAutoType(const PwAutoType& original);
    virtual ~PwAutoType();

    void clear();

    PwAutoType* clone() const;

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml);
    void writeToStream(QXmlStreamWriter& xml) const;
};

class PwDatabaseV4;

/**
 * Database V4 entry
 */
class PwEntryV4: public PwEntry {
    Q_OBJECT
    Q_PROPERTY(PwUuid customIconUuid READ getCustomIconUuid WRITE setCustomIconUuid NOTIFY customIconUuidChanged)
    Q_PROPERTY(int extraSize READ getExtraSize NOTIFY extraSizeChanged)
    Q_PROPERTY(int historySize READ getHistorySize NOTIFY historySizeChanged)
private:
    PwUuid _customIconUuid;
    PwAutoType _autoType;
    QMap<QString, PwField*> fields;
    bb::cascades::QListDataModel<PwField*> _extraFieldsDataModel;
    bb::cascades::QListDataModel<PwEntryV4*> _historyDataModel;
    quint32 _usageCount;
    QDateTime _locationChangedTime;

    QString _foregroundColor;
    QString _backgroundColor;
    QString _overrideUrl;
    QString _tags;

    // Loads timestamps of an entry
    ErrorCodesV4::ErrorCode readTimes(QXmlStreamReader& xml);
    // Loads the history tag of an entry and fills entry's history list
    ErrorCodesV4::ErrorCode readHistory(QXmlStreamReader& xml, const PwMetaV4& meta, Salsa20& salsa20);
    // Loads an entry's binary attachment ("Binary" field of an entry).
    ErrorCodesV4::ErrorCode readAttachment(QXmlStreamReader& xml, const PwMetaV4& meta, Salsa20& salsa20, PwAttachment& attachment);
    // Writes all entry's attachments to an XML stream.
    void writeAttachments(QXmlStreamWriter& xml);

    /**
     * Adds a named field value to the entry.
     */
    void addField(PwField* field);
    /**
     * Updates a named field's value (adding if necessary).
     */
    void setField(const QString& name, const QString& value);

    void addHistoryEntry(PwEntryV4* historyEntry);
public:
    PwEntryV4(QObject* parent=0);
    virtual ~PwEntryV4();

    virtual void clear();

    /** Returns a new entry instance with the same field values */
    virtual PwEntry* clone();

    /** Search helper. Returns true if any of the fields contain the query string. */
    virtual bool matchesQuery(const QString& query) const;

    /**
     * Makes a backup copy of the current values/state of the entry.
     * (For V4 adds the current state to entry's history)
     * Returns true if successful.
     */
    virtual bool backupState();

    /** Updates last access timestamp to current time and increases usage counter */
    virtual void registerAccessEvent();

    /**
     * Loads the given file and attaches it to the entry.
     * Makes a backup of the initial entry state.
     * Returns true if successful, false in case of any error.
     */
    virtual bool attachFile(const QString& filePath);

    /**
     * Loads entry fields from the stream.
     * The caller is responsible for clearing any previous values.
     */
    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml, const PwMetaV4& meta, Salsa20& salsa20);
    /**
     * Writes the entry to the stream.
     */
    void writeToStream(QXmlStreamWriter& xml, PwMetaV4& meta, Salsa20& salsa20);

    Q_INVOKABLE bb::cascades::DataModel* getExtraFieldsDataModel();
    Q_INVOKABLE bb::cascades::DataModel* getHistoryDataModel();

    // overriden property accessors
    virtual QString getTitle() const;
    virtual void setTitle(const QString& title);
    virtual QString getUserName() const;
    virtual void setUserName(const QString& userName);
    virtual QString getPassword() const;
    virtual void setPassword(const QString& password);
    virtual QString getUrl() const;
    virtual void setUrl(const QString& url);
    virtual QString getNotes() const;
    virtual void setNotes(const QString& notes);

    // own property accessors
    PwUuid getCustomIconUuid() const { return _customIconUuid; }
    void setCustomIconUuid(const PwUuid& uuid);
    int getHistorySize() { return _historyDataModel.size(); }
    int getExtraSize() { return _extraFieldsDataModel.size(); }
    quint32 getUsageCount() const { return _usageCount; }
    void setUsageCount(const quint32 usageCount);
    QDateTime getLocationChangedTime() const { return _locationChangedTime; }
    void setLocationChangedTime(const QDateTime& locationChangedTime);
    QString getForegroundColor() const { return _foregroundColor; }
    void setForegroundColor(const QString& fgColor);
    QString getBackgroundColor() const { return _backgroundColor; }
    void setBackgroundColor(const QString& bgColor);
    QString getOverrideUrl() const { return _overrideUrl; }
    void setOverrideUrl(const QString& url);
    QString getTags() const { return _tags; }
    void setTags(const QString& tags);
signals:
    void customIconUuidChanged(PwUuid);
    void historySizeChanged(int);
    void extraSizeChanged(int);
    void usageCountChanged(quint32);
    void locationChangedTimeChanged(QDateTime);
    void foregroundColorChanged(QString);
    void backgroundColorChanged(QString);
    void overrideUrlChanged(QString);
    void tagsChanged(QString);
};

Q_DECLARE_METATYPE(PwField*);
Q_DECLARE_METATYPE(PwEntryV4*);

#endif /* PWENTRYV4_H_ */
