/*
 * PwMetaV4.h
 *
 *  Created on: 11 Dec 2014
 *      Author: Andrei Popleteev
 */

#ifndef PWMETAV4_H_
#define PWMETAV4_H_

#include <QObject>
#include <QDateTime>
#include <QtXml/QXmlStreamReader>
#include "db/PwUuid.h"
#include "db/v4/DefsV4.h"
#include "crypto/CryptoManager.h"


/**
 * Memory protection configuration of a V4 database.
 */
class MemoryProtection: public QObject {
    Q_OBJECT
private:
    bool protectTitle;
    bool protectUserName;
    bool protectPassword;
    bool protectUrl;
    bool protectNotes;

public:
    MemoryProtection(QObject* parent=0);
    virtual ~MemoryProtection();

    void clear();
    QString toString() const;

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml);
    void writeToStream(QXmlStreamWriter& xml);
};

/**
 * Binary data as stored in DB metadata
 */
class PwBinaryV4: public QObject {
    Q_OBJECT
private:
    QString _id;
    bool _isCompressed;
    QByteArray _data;
public:
    PwBinaryV4(QObject* parent=0);
    virtual ~PwBinaryV4();

    void clear();
    QString toString() const;

    bool readFromStream(QXmlStreamReader& xml);
    void writeToStream(QXmlStreamWriter& xml);

    QString getId() const { return _id; }
    bool isCompressed() const { return _isCompressed; }
    QByteArray getData() const { return _data; }
};

/**
 * Custom icon in a V4 database
 */
class PwCustomIconV4: public QObject {
    Q_OBJECT
private:
    PwUuid uuid;
    QByteArray data;
public:
    PwCustomIconV4(QObject* parent=0);
    virtual ~PwCustomIconV4();

    void clear();
    QString toString() const;

    /** Reads icon fields from an XML stream. Returns true if successful, false otherwise. */
    bool readFromStream(QXmlStreamReader& xml);
    /** Writes icon fields to an XML stream. */
    void writeToStream(QXmlStreamWriter& xml);

    PwUuid getUuid() const;
    QByteArray getData() const;
};

/**
 * Metadata of the V4 database
 */
class PwMetaV4: public QObject {
    Q_OBJECT
private:
    QString generator;
    QByteArray headerHash;
    QString databaseName;
    QDateTime databaseNameChangedTime;
    QString databaseDescription;
    QDateTime databaseDescriptionChangedTime;
    QString defaultUserName;
    QDateTime defaultUserNameChangedTime;
    quint32 maintenanceHistoryDays;
    QString colorString; // database color coded as a CSS-format hex string(e.g. #123456), empty string means transparent
    QDateTime masterKeyChangedTime;
    qint64 masterKeyChangeRec;
    qint64 masterKeyChangeForce;
    MemoryProtection memoryProtection;
    bool recycleBinEnabled;
    PwUuid recycleBinGroupUuid;
    QDateTime recycleBinChangedTime;
    PwUuid entryTemplatesGroupUuid;
    QDateTime entryTemplatesGroupChangedTime;
    qint32 historyMaxItems;
    qint64 historyMaxSize;
    PwUuid lastSelectedGroupUuid;
    PwUuid lastTopVisibleGroupUuid;
    QMap<QString, QString> customData; // a set of key=value pairs

    QMap<PwUuid, PwCustomIconV4*> customIcons;
    QMap<QString, PwBinaryV4*> binaries;

    ErrorCodesV4::ErrorCode readCustomData(QXmlStreamReader& xml);
    ErrorCodesV4::ErrorCode readCustomDataItem(QXmlStreamReader& xml);
    ErrorCodesV4::ErrorCode readBinaries(QXmlStreamReader& xml);
    ErrorCodesV4::ErrorCode readCustomIcons(QXmlStreamReader& xml);

    void writeCustomData(QXmlStreamWriter& xml) const;
    void writeBinaries(QXmlStreamWriter& xml) const;
    void writeCustomIcons(QXmlStreamWriter& xml) const;

public:
    void debugPrint() const;
    PwMetaV4(QObject* parent=0);
    virtual ~PwMetaV4();

    void clear();

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml);
    ErrorCodesV4::ErrorCode writeToStream(QXmlStreamWriter& xml);

    PwBinaryV4* getBinaryByReference(const QString& ref) const;

    /**
     * Checks if the hashes DB file's header hash matches the one specified in Meta data, if any.
     * If no header hash was specified in Meta, returns true.
     */
    bool isHeaderHashMatch(const QByteArray& dbHeaderHash) const;

    // proprety accessors
    bool isRecycleBinEnabled() const { return recycleBinEnabled; }
    PwUuid getRecycleBinGroupUuid() const { return recycleBinGroupUuid; }
};


#endif /* PWMETAV4_H_ */
