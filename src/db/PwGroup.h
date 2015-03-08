/*
 * PwGroup.h
 *
 *  Created on: 26 May 2014
 *      Author: andrei.popleteev
 */

#ifndef PWGROUP_H_
#define PWGROUP_H_

#include <QObject>
#include <QMetaType>
#include <QList>
#include <bb/cascades/DataModel>
#include "db/PwUuid.h"

class PwEntry;
class PwDatabase;

using namespace bb::cascades;

/**
 * Parameters for search/filtering
 */
struct SearchParams {
    bool includeSubgroups;
    bool includeDeleted;
    QString query;
};

class PwGroup : public bb::cascades::DataModel {
	Q_OBJECT
	Q_PROPERTY(int iconId READ getIconId WRITE setIconId NOTIFY iconIdChanged)
	Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString notes READ getNotes WRITE setNotes NOTIFY notesChanged)
	Q_PROPERTY(int itemsCount READ immediateChildCount NOTIFY itemsCountChanged)
    Q_PROPERTY(QDateTime creationTime READ getCreationTime WRITE setCreationTime NOTIFY creationTimeChanged)
    Q_PROPERTY(QDateTime lastModificationTime READ getLastModificationTime WRITE setLastModificationTime NOTIFY lastModificationTimeChanged)
    Q_PROPERTY(QDateTime lastAccessTime READ getLastAccessTime WRITE setLastAccessTime NOTIFY lastAccessTimeChanged)
    Q_PROPERTY(QDateTime expiryTime READ getExpiryTime WRITE setExpiryTime NOTIFY expiryTimeChanged)
    Q_PROPERTY(bool expires READ isExpires WRITE setExpires NOTIFY expiresChanged)
    // indicates whether the group is in Recycle Bin
    Q_PROPERTY(bool deleted READ isDeleted NOTIFY deletedChanged)
    Q_PROPERTY(PwGroup* parentGroup READ getParentGroup WRITE setParentGroup NOTIFY parentGroupChanged)

    friend class PwDatabase;

private:
    PwDatabase* _database;
	PwUuid _uuid;
	int _iconId;
	QString _name;
	QString _notes;
    QDateTime _creationTime;
    QDateTime _lastModificationTime;
    QDateTime _lastAccessTime;
    QDateTime _expiryTime;
    bool _expires;
    bool _deleted;

	bool _isChildrenModified;
	QList<PwGroup*> _subGroups, sortedGroups;
	QList<PwEntry*> _entries, sortedEntries;
	PwGroup* _parentGroup;
private slots:
	// Relays itemsChanged to itemsCountChanged; needed to match signatures of the two signals.
	void itemsCountChangedAdapter(bb::cascades::DataModelChangeType::Type changeType);
protected slots:
	/**
	 * Sorts subgroups and entries according to the settings
	 */
	virtual void sortChildren();
public:
    static const int DEFAULT_ICON_ID;

	PwGroup(QObject* parent=0);
	virtual ~PwGroup();

	virtual void clear();

    /**
     * Recursively iterates through all the children groups and entries of this group
     * and adds them to the given lists. The group itself is excluded.
     */
    virtual void getAllChildren(QList<PwGroup*> &childGroups, QList<PwEntry*> &childEntries) const;

    /** Removes the group from the parent group, if any. Does NOT make a copy in Backup/Recycle bin. */
    Q_INVOKABLE void deleteWithoutBackup();

    /**
     * Moves the group and all of its children to Backup group.
     * (Exact behaviour is DB-version specific).
     * Returns true if successful.
     */
    Q_INVOKABLE virtual bool moveToBackup() { /* stub to avoid pure abstract PwGroup */ return false; }

	virtual void addSubGroup(PwGroup* subGroup);
	virtual void removeSubGroup(PwGroup* subGroup);
	QList<PwGroup*> getSubGroups() const { return _subGroups; }

	virtual void addEntry(PwEntry* entry);
	virtual void removeEntry(PwEntry* entry);
	QList<PwEntry*> getEntries() const { return _entries; }

	/**
	 * Checks if a group name is reserved for internal use and cannot be assigned by the user.
	 */
	Q_INVOKABLE virtual bool isNameReserved(const QString& name) { Q_UNUSED(name); return false; }

    /**
     * Creates an entry in the group and returns a reference to it.
     * (This method should actually be pure virtual, but that makes PwGroup abstract and which causes problems in QML.
     *  So PwGroup's implementation simply returns null; subclasses should return appropriate versions of entry instances.)
     */
    Q_INVOKABLE virtual PwEntry* createEntry() { return NULL; }
    /**
     * Creates a subgroup in the group and returns a reference to it.
     * (This method should actually be pure virtual, but that makes PwGroup abstract and which causes problems in QML.
     *  So PwGroup's implementation simply returns null; subclasses should return appropriate versions of group instances.)
     */
    Q_INVOKABLE virtual PwGroup* createGroup() { return NULL; }

    /**
     * Returns true if the group has expired.
     */
    Q_INVOKABLE virtual bool isExpired() const;

    /** Updates last access timestamp to current time */
    Q_INVOKABLE virtual void registerAccessEvent();
    /** Updates modification (and last access) timestamp to current time */
    Q_INVOKABLE virtual void registerModificationEvent();

    /**
     * Returns the (sub)group with the given UUID (searching the full tree),
     * or NULL if no such group found.
     */
    virtual PwGroup* findGroupByUuid(const PwUuid& uuid) const;
    /**
     * Finds entries which contain the query substring, and adds them to the result.
     */
    virtual void filterEntries(const SearchParams& params, QList<PwEntry*> &result) const;

    // DataModel interface implementation
    Q_INVOKABLE virtual int childCount(const QVariantList& indexPath);
    Q_INVOKABLE virtual bool hasChildren(const QVariantList& indexPath);
    Q_INVOKABLE virtual QVariant data(const QVariantList& indexPath);
    Q_INVOKABLE virtual QString itemType(const QVariantList& indexPath);

    // property getters/setters
    int immediateChildCount() const { return _subGroups.count() + _entries.count(); }
    PwUuid getUuid() const { return _uuid; }
    void setUuid(const PwUuid& uuid);
    int getIconId() const { return _iconId; }
    void setIconId(int iconId);
    QString getName() const { return _name; }
    void setName(const QString& name);
    QString getNotes() const { return _notes; }
    void setNotes(const QString& notes);
    QDateTime getCreationTime() const { return _creationTime; }
    void setCreationTime(const QDateTime& time);
    QDateTime getLastModificationTime() const { return _lastModificationTime; }
    void setLastModificationTime(const QDateTime& time);
    QDateTime getLastAccessTime() const { return _lastAccessTime; }
    void setLastAccessTime(const QDateTime& time);
    QDateTime getExpiryTime() const { return _expiryTime; }
    void setExpiryTime(const QDateTime& time);
    bool isExpires() const { return _expires; }
    void setExpires(bool expires);
    bool isDeleted() const { return _deleted; }
    void setDeleted(bool deleted);
    PwGroup* getParentGroup() const { return _parentGroup; }
    void setParentGroup(PwGroup* parentGroup);
    void setDatabase(PwDatabase* database);
    PwDatabase* getDatabase() const { return _database; }

    /** Returns a string representation of the instance */
    virtual QString toString() const;
    /** Comparator for sorting */
    static bool lessThan(const PwGroup* g1, const PwGroup* g2);
signals:
    void uuidChanged(PwUuid);
    void iconIdChanged(int);
    void nameChanged(QString);
    void notesChanged(QString);
    void itemsCountChanged(int);
    void creationTimeChanged(QDateTime);
    void lastModificationTimeChanged(QDateTime);
    void lastAccessTimeChanged(QDateTime);
    void expiryTimeChanged(QDateTime);
    void expiresChanged(bool);
    void deletedChanged(bool);
    void parentGroupChanged(PwGroup*);
    void databaseChanged(PwDatabase*);
};

Q_DECLARE_METATYPE(PwGroup*);

#endif /* PWGROUP_H_ */
