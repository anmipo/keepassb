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
#include <bb/cascades/DataModel>
#include <bb/cascades/QListDataModel>

using namespace bb::cascades;

/**
 * Binary attachment of a database entry
 */
class PwAttachment: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(int size READ getSize NOTIFY sizeChanged)
private:
    bool isInitialized;
    QString name;
    bool isOriginallyCompressed; // 'compressed' flag of the original data
    bool isCompressed;  // lazy-uncompress helper flag
    QByteArray data;

    /**
     * Unpacks content.data if it is compressed.
     * Returns true if successful.
     */
    bool inflateData();
public:
    PwAttachment(QObject* parent=0);
    virtual ~PwAttachment();

    /** Returns true if any string contains the query string. */
    virtual bool matchesQuery(const QString& query) const;

    /**
     * Loads the given file and returns the corresponding PwAttachment instance.
     * In case of error returns NULL.
     */
    static PwAttachment* createFromFile(const QString& filePath);

    /**
     * Stores attachment contents to the specified file (creates or overwrites as necessary)
     */
    Q_INVOKABLE bool saveContentToFile(const QString& fileName);

    /** Sets attachment content */
    void setData(const QByteArray& data, const bool isCompressed);
    QByteArray getData() const { return data; }

    void clear();

    /**
     * Returns true if neither name nor data have been set.
     */
    bool isEmpty() const { return !isInitialized; }

    // property accessors
    void setName(const QString& name);
    QString getName() const { return name; }
    int getSize();
signals:
    void nameChanged(QString);
    void sizeChanged(int);
};

/*****************************/

class PwAttachmentDataModel: public bb::cascades::QListDataModel<PwAttachment*> {
    Q_OBJECT
    Q_PROPERTY(int size READ size NOTIFY sizeChanged)
private:
    int _size;
private slots:
    void updateSize();
public:
    PwAttachmentDataModel(QObject* parent=0);
    virtual ~PwAttachmentDataModel() {}

    /**
     * Clears and disposes of each attachment before calling base class' method.
     */
    void clear();

    // by default QListDataModel's removeAt is not available to QML, so we expose it
    Q_INVOKABLE void removeAt(int index);

signals:
    void sizeChanged(int);
};

/*****************************/

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
	Q_PROPERTY(bool expires READ isExpires WRITE setExpires NOTIFY expiresChanged)
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
    bool _expires;
    bool _deleted;
    PwGroup* _parentGroup;
    PwAttachmentDataModel _attachmentsDataModel;

protected:
    // const pointer to attachments' data model for child classes
    const PwAttachmentDataModel* getConstAttachmentsDataModel() const { return &_attachmentsDataModel; }

public:
    static const int DEFAULT_ICON_ID;

	PwEntry(QObject* parent=0);
	virtual ~PwEntry();

	virtual void clear();

    /** Removes the entry from the parent group. Does NOT make a copy in Backup/Recycle bin. */
    Q_INVOKABLE void deleteWithoutBackup();

    /** Adds an attachment, if possible. Returns true if successful. */
	virtual bool addAttachment(PwAttachment* attachment);

	/**
	 * Loads the given file and attaches it to the entry.
     * Makes a backup of the initial entry state.
	 * Returns true if successful, false in case of any error.
	 */
	Q_INVOKABLE virtual bool attachFile(const QString& filePath) = 0;

    Q_INVOKABLE PwAttachmentDataModel* getAttachmentsDataModel() { return &_attachmentsDataModel; }

    /** Returns a new entry instance with the same field values */
    virtual PwEntry* clone() = 0;

    /**
     * Makes a backup copy of the current values/state of the entry.
     * Actual behaviour is DB version-specific.
     * Returns true if successful.
     */
    Q_INVOKABLE virtual bool backupState() = 0;

    /**
     * Moves the entry to Backup/Recycle group.
     * Returns true if successful.
     */
    Q_INVOKABLE virtual bool moveToBackup();

    /**
     * Returns true if the entry has expired.
     */
    Q_INVOKABLE virtual bool isExpired() const;

    /** Updates modification and last access timestamps to current time */
    Q_INVOKABLE void renewTimestamps();

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
    virtual QDateTime getExpiryTime() const { return _expiryTime; }
    virtual void setExpiryTime(const QDateTime& time);
    virtual bool isExpires() const { return _expires; }
    virtual void setExpires(bool expires);
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
    virtual bool matchesQuery(const QString& query) const;
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
    void expiresChanged(bool);
    void deletedChanged(bool);
    void parentGroupChanged(PwGroup*);
};

Q_DECLARE_METATYPE(PwAttachment*);
Q_DECLARE_METATYPE(PwAttachmentDataModel*);
Q_DECLARE_METATYPE(PwEntry*);

#endif /* PWENTRY_H_ */
