/*
 * PwEntryV3.h
 *
 *  Created on: 21 Aug 2014
 *      Author: Andrei
 */

#ifndef PWENTRYV3_H_
#define PWENTRYV3_H_

#include "db/PwEntry.h"

/**
 * Database v3 entry
 */
class PwEntryV3: public PwEntry {
    Q_OBJECT
    Q_PROPERTY(qint32 groupId READ getGroupId WRITE setGroupId NOTIFY groupIdChanged)
private:
    qint32 _groupId;
    QString _title;
    QString _userName;
    QString _password;
    QString _url;
    QString _notes;

    const static quint16 FIELD_RESERVED  = 0x0000;
    const static quint16 FIELD_UUID      = 0x0001;
    const static quint16 FIELD_GROUP_ID  = 0x0002;
    const static quint16 FIELD_ICON_ID   = 0x0003;
    const static quint16 FIELD_TITLE     = 0x0004;
    const static quint16 FIELD_URL       = 0x0005;
    const static quint16 FIELD_USERNAME  = 0x0006;
    const static quint16 FIELD_PASSWORD  = 0x0007;
    const static quint16 FIELD_NOTE      = 0x0008;
    const static quint16 FIELD_CREATION_TIME      = 0x0009;
    const static quint16 FIELD_LAST_MODIFIED_TIME = 0x000A;
    const static quint16 FIELD_LAST_ACCESS_TIME   = 0x000B;
    const static quint16 FIELD_EXPIRATION_TIME    = 0x000C;
    const static quint16 FIELD_BINARY_DESC        = 0x000D;
    const static quint16 FIELD_BINARY_DATA        = 0x000E;
    const static quint16 FIELD_END                = 0xFFFF;

    /** Returns a PwAttachment object for this entry. If there is none, returns NULL. */
    PwAttachment* getAttachment() const;

public:
    PwEntryV3(QObject* parent=0);
    virtual ~PwEntryV3();

    /** Loads entry fields from the stream. Returns true on success, false in case of error. */
    bool readFromStream(QDataStream& stream);
    /** Writes entry fields to the stream. Returns true on success, false in case of error. */
    bool writeToStream(QDataStream& stream);

    virtual void clear();

    /** Search helper. Returns true if any of the fields contain the query string. */
    virtual bool matchesQuery(const QString& query) const;

    /** Returns a new entry instance with the same field values */
    virtual PwEntry* clone();

    /** Adds an attachment (if there is none already). Returns true if successful. */
    virtual bool addAttachment(PwAttachment* attachment);

    /** Identifies if this entry is a special internal meta-stream data */
    bool isMetaStream() const;

    /**
     * Makes a backup copy of the current values/state of the entry.
     * (For V3 is equivalent to backupEntry())
     * Returns true if successful.
     */
    virtual bool backupState();

    // property accessors
    qint32 getGroupId() const { return _groupId; }
    void setGroupId(qint32 groupId);
    virtual QString getTitle() const { return _title; }
    virtual void setTitle(const QString& title);
    virtual QString getUserName() const { return _userName; }
    virtual void setUserName(const QString& userName);
    virtual QString getPassword() const { return _password; }
    virtual void setPassword(const QString& password);
    virtual QString getUrl() const { return _url; }
    virtual void setUrl(const QString& url);
    virtual QString getNotes() const { return _notes; }
    virtual void setNotes(const QString& notes);
    // override to handle the "Never" magic const
    virtual void setExpiryTime(const QDateTime& time);
signals:
    void groupIdChanged(qint32 groupId);
    // other ***Changed signals are defined in the parent class
};

#endif /* PWENTRYV3_H_ */
