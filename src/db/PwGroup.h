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

class PwEntry;

class PwGroup : public bb::cascades::DataModel {
	Q_OBJECT
	Q_PROPERTY(int iconId READ getIconId WRITE setIconId NOTIFY iconIdChanged)
	Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString notes READ getNotes WRITE setNotes NOTIFY notesChanged)
	Q_PROPERTY(int childCount READ immediateChildCount NOTIFY childCountChanged)
    Q_PROPERTY(QDateTime creationTime READ getCreationTime WRITE setCreationTime NOTIFY creationTimeChanged)
    Q_PROPERTY(QDateTime lastModificationTime READ getLastModificationTime WRITE setLastModificationTime NOTIFY lastModificationTimeChanged)
    Q_PROPERTY(QDateTime lastAccessTime READ getLastAccessTime WRITE setLastAccessTime NOTIFY lastAccessTimeChanged)
    Q_PROPERTY(QDateTime expiryTime READ getExpiryTime WRITE setExpiryTime NOTIFY expiryTimeChanged)
private:
	QByteArray _uuid;
	int _iconId;
	QString _name;
	QString _notes;
    QDateTime _creationTime;
    QDateTime _lastModificationTime;
    QDateTime _lastAccessTime;
    QDateTime _expiryTime;

	bool _isChildrenModified;
	QList<PwGroup*> _subGroups;
	QList<PwEntry*> _entries;

protected:
	/**
	 * Sorts subgroups and entries alphabetically
	 */
	virtual void sortChildren();
public:
	PwGroup();
	virtual ~PwGroup();

	virtual void clear();

	void addSubGroup(PwGroup* subGroup);
	QList<PwGroup*> getSubGroups() const { return _subGroups; }

	void addEntry(PwEntry* entry);
	QList<PwEntry*> getEntries() const { return _entries; }
    Q_INVOKABLE bool isEmpty() const { return immediateChildCount() == 0;};

    /**
     * Finds entries which contain the query substring, and adds them to the result.
     */
    void filterEntries(const QString& query, QList<PwEntry*> &result, bool includeSubgroups) const;

    // DataModel interface implementation
    Q_INVOKABLE virtual int childCount(const QVariantList& indexPath);
    Q_INVOKABLE virtual bool hasChildren(const QVariantList& indexPath);
    Q_INVOKABLE virtual QVariant data(const QVariantList& indexPath);
    Q_INVOKABLE virtual QString itemType(const QVariantList& indexPath);

    // property getters/setters
    int immediateChildCount() const { return _subGroups.count() + _entries.count(); }
    QByteArray getUuid() const { return _uuid; }
    void setUuid(const QByteArray& uuid) { _uuid = uuid; }
    int getIconId() const { return _iconId; }
    void setIconId(int iconId) { _iconId = iconId; }
    QString getName() const { return _name; }
    void setName(const QString& name) { _name = name; }
    QString getNotes() const { return _notes; }
    void setNotes(const QString& notes) { _notes = notes; }
    QDateTime getCreationTime() const { return _creationTime; }
    void setCreationTime(const QDateTime& time) { _creationTime = time; }
    QDateTime getLastModificationTime() const { return _lastModificationTime; }
    void setLastModificationTime(const QDateTime& time) { _lastModificationTime = time; }
    QDateTime getLastAccessTime() const { return _lastAccessTime; }
    void setLastAccessTime(const QDateTime& time) { _lastAccessTime = time; }
    QDateTime getExpiryTime() const { return _expiryTime; }
    void setExpiryTime(const QDateTime& time) { _expiryTime = time; }

    /** Returns a string representation of the instance */
    virtual QString toString() const;
    /** Comparator for sorting */
    static bool lessThan(const PwGroup* g1, const PwGroup* g2);
signals:
    void iconIdChanged(int);
    void nameChanged(QString);
    void notesChanged(QString);
    void childCountChanged(int);
    void creationTimeChanged(QDateTime);
    void lastModificationTimeChanged(QDateTime);
    void lastAccessTimeChanged(QDateTime);
    void expiryTimeChanged(QDateTime);
};

Q_DECLARE_METATYPE(PwGroup*);

#endif /* PWGROUP_H_ */
