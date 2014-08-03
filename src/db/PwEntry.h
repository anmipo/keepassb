/*
 * PwEntry.h
 *
 *  Created on: 27 May 2014
 *      Author: Andrei
 */

#ifndef PWENTRY_H_
#define PWENTRY_H_

#include <QObject>
#include <QMetaType>
#include <QDateTime>
#include "db/PwUuid.h"
#include "db/PwGroup.h"

class PwEntry: public QObject {
	Q_OBJECT
	Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
	Q_PROPERTY(int iconId READ getIconId WRITE setIconId NOTIFY iconIdChanged)
	Q_PROPERTY(QString userName READ getUserName WRITE setUserName NOTIFY userNameChanged)
	Q_PROPERTY(QString password READ getPassword WRITE setPassword NOTIFY passwordChanged)
	Q_PROPERTY(QString url READ getUrl WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(QString notes READ getNotes WRITE setNotes NOTIFY notesChanged)
	Q_PROPERTY(QDateTime creationTime READ getCreationTime WRITE setCreationTime NOTIFY creationTimeChanged)
	Q_PROPERTY(QDateTime lastModificationTime READ getLastModificationTime WRITE setLastModificationTime NOTIFY lastModificationTimeChanged)
	Q_PROPERTY(QDateTime lastAccessTime READ getLastAccessTime WRITE setLastAccessTime NOTIFY lastAccessTimeChanged)
	Q_PROPERTY(QDateTime expiryTime READ getExpiryTime WRITE setExpiryTime NOTIFY expiryTimeChanged)
    // indicates whether the entry is in Recycle Bin
    Q_PROPERTY(bool deleted READ isDeleted NOTIFY deletedChanged)
    Q_PROPERTY(PwGroup* parentGroup READ getParentGroup WRITE setParentGroup NOTIFY parentGroupChanged)
private:
	PwUuid _uuid;
	int _iconId;
    QDateTime _creationTime;
    QDateTime _lastModificationTime;
    QDateTime _lastAccessTime;
    QDateTime _expiryTime;
    bool _deleted;
    PwGroup* _parentGroup;

public:
	PwEntry(QObject* parent=0);
	virtual ~PwEntry();

	virtual void clear();


	// property getters/setters
	PwUuid getUuid() const { return _uuid; }
	void setUuid(const PwUuid& uuid);
	virtual int getIconId() const { return _iconId; }
	virtual void setIconId(int iconId);
    QDateTime getCreationTime() const { return _creationTime; }
    void setCreationTime(const QDateTime& time);
    QDateTime getLastModificationTime() const { return _lastModificationTime; }
    void setLastModificationTime(const QDateTime& time);
    QDateTime getLastAccessTime() const { return _lastAccessTime; }
    void setLastAccessTime(const QDateTime& time);
    QDateTime getExpiryTime() const { return _expiryTime; }
    void setExpiryTime(const QDateTime& time);
    bool isDeleted() const { return _deleted; }
    void setDeleted(bool deleted);
    PwGroup* getParentGroup() const { return _parentGroup; }
    void setParentGroup(PwGroup* parentGroup);
	// pure virtual getters/setters
	virtual QString getTitle() const = 0;
	virtual void setTitle(const QString& title) = 0;
	virtual QString getUserName() const = 0;
	virtual void setUserName(const QString& userName) = 0;
	virtual QString getPassword() const = 0;
	virtual void setPassword(const QString& password) = 0;
	virtual QString getUrl() const = 0;
	virtual void setUrl(const QString& url) = 0;
	virtual QString getNotes() const = 0;
	virtual void setNotes(const QString& notes) = 0;

    /** Returns a string representation of the instance */
    virtual QString toString() const;
    /** Comparator for sorting */
    static bool lessThan(const PwEntry* e1, const PwEntry* e2);

    /** Search helper. Returns true if any of the fields contain the query string. */
    virtual bool matchesQuery(const QString& query) const = 0;
signals:
    void uuidChanged(PwUuid);
    void titleChanged(QString);
    void iconIdChanged(int);
    void userNameChanged(QString);
    void passwordChanged(QString);
    void urlChanged(QString);
    void notesChanged(QString);
    void creationTimeChanged(QDateTime);
    void lastModificationTimeChanged(QDateTime);
    void lastAccessTimeChanged(QDateTime);
    void expiryTimeChanged(QDateTime);
    void deletedChanged(bool);
    void parentGroupChanged(PwGroup*);
};

Q_DECLARE_METATYPE(PwEntry*);

#endif /* PWENTRY_H_ */
