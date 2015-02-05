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
 * Extra (string) field of a V4 database entry
 */
class PwExtraField: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(QString value READ getValue NOTIFY valueChanged)
private:
    QString _name;
    QString _value;
public:
    PwExtraField(QObject* parent=0);
    PwExtraField(QObject* parent, const QString& name, const QString& value);
    virtual ~PwExtraField();

    /** Returns true if any string contains the query string. */
    virtual bool matchesQuery(const QString& query) const;

    Q_INVOKABLE QString toString() const;

    // property accessors
    QString getName() const { return _name; }
    QString getValue() const { return _value; }
signals:
    // nameChanged and valueChanged are never emitted
    void nameChanged(QString name);
    void valueChanged(QString value);
};

/**
 * Auto type settings of a V4 entry
 */
class PwAutoType: public QObject {
    Q_OBJECT
private:
    bool _enabled;
    quint32 _obfuscationType;
    QString _defaultSequence;
    QList<QStringPair> _associations;

    ErrorCodesV4::ErrorCode readAssociation(QXmlStreamReader& xml);
public:
    PwAutoType(QObject* parent=0);
    virtual ~PwAutoType();

    void clear();

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml);
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
    QMap<QString, QString> fields;
    bb::cascades::QListDataModel<PwExtraField*> _extraFieldsDataModel;
    bb::cascades::QListDataModel<PwEntryV4*> _historyDataModel;
    quint32 _usageCount;
    QDateTime _locationChangedTime;

    QString _foregroundColor;
    QString _backgroundColor;
    QString _overrideUrl;
    QString _tags;

    bool isStandardField(const QString& name) const;

    // Loads timestamps of an entry
    ErrorCodesV4::ErrorCode readTimes(QXmlStreamReader& xml);
    // Loads the history tag of an entry and fills entry's history list
    ErrorCodesV4::ErrorCode readHistory(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20);
    // Loads a "String" field of an entry.
    ErrorCodesV4::ErrorCode readString(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20);
    // Loads the value of a "String" field of an entry; decrypts protected values.
    ErrorCodesV4::ErrorCode readStringValue(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20, QString& value);
    // Loads an entry's binary attachment ("Binary" field of an entry).
    ErrorCodesV4::ErrorCode readAttachment(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20, PwAttachment& attachment);

public:
    PwEntryV4(QObject* parent=0);
    virtual ~PwEntryV4();

    virtual void clear();

    /**
     * Loads entry fields from the stream.
     * The caller is responsible for clearing any previous values.
     */
    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20);

    /** Search helper. Returns true if any of the fields contain the query string. */
    virtual bool matchesQuery(const QString& query) const;

    /** Returns a new entry instance with the same field values */
    virtual PwEntry* clone();

    /**
     * Makes a backup copy of the current values/state of the entry.
     * (For V4 adds the current state to entry's history)
     * Returns true if successful.
     */
    virtual bool backupState();

    /**
     * Loads the given file and attaches it to the entry.
     * Makes a backup of the initial entry state.
     * Returns true if successful, false in case of any error.
     */
    virtual bool attachFile(const QString& filePath);

    void addExtraField(const QString& name, const QString& value);
    void addHistoryEntry(PwEntryV4* historyEntry);

    /**
     * Adds a named field value to the entry.
     */
    void setField(const QString& name, const QString& value);

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

Q_DECLARE_METATYPE(PwExtraField*);
Q_DECLARE_METATYPE(PwEntryV4*);

#endif /* PWENTRYV4_H_ */
