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
    QString _binaryDesc;
    QByteArray _binaryData;
    QString _title;
    QString _userName;
    QString _password;
    QString _url;
    QString _notes;
public:
    PwEntryV3(QObject* parent=0);
    virtual ~PwEntryV3();

    virtual void clear();

    /** Search helper. Returns true if any of the fields contain the query string. */
    virtual bool matchesQuery(const QString& query) const;

    virtual void addAttachment(PwAttachment* attachment);

    /** Identifies if this entry is a special internal meta-stream data */
    bool isMetaStream() const;

    // property accessors
    qint32 getGroupId() const { return _groupId; }
    void setGroupId(qint32 groupId);
    QString getBinaryDesc() const { return _binaryDesc; }
    void setBinaryDesc(QString desc);
    QByteArray getBinaryData() const { return _binaryData; }
    void setBinaryData(const QByteArray& data);
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
    void binaryDescChanged(QString binaryDesc);
    void binaryDataChanged(QByteArray binaryData);
    // other ***Changed signals are defined in the parent class
};

#endif /* PWENTRYV3_H_ */
