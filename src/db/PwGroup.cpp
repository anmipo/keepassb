/*
 * PwGroup.cpp
 *
 *  Created on: 26 May 2014
 *      Author: andrei.popleteev
 */

#include "PwGroup.h"
#include "PwEntry.h"
#include <QDebug>
#include <QStringBuilder>
#include <bb/cascades/ItemGrouping>
#include <bb/cascades/DataModel>
#include <bb/cascades/DataModelChangeType>
#include "db/PwDatabase.h"
#include "util/Settings.h"
#include "util/Util.h"

const int PwGroup::DEFAULT_ICON_ID = 48; // "folder"
const int PwGroup::DEFAULT_OPEN_ICON_ID = 49; // "open folder"

PwGroup::PwGroup(QObject* parent) :
        bb::cascades::DataModel(parent), _database(NULL), _uuid(),
        _creationTime(), _lastModificationTime(),
        _lastAccessTime(), _expiryTime(),
        _subGroups(), sortedGroups(),
        _entries(), sortedEntries() {
	_name = QString::null;
	_notes = QString::null;
	_iconId = 0;
	_isChildrenModified = false;
	_expires = false;
	_deleted = false;
	_parentGroup = NULL;

	bool res = QObject::connect(Settings::instance(), SIGNAL(alphaSortingChanged(bool)), this, SLOT(sortChildren())); Q_ASSERT(res);
	res = QObject::connect(this, SIGNAL(itemsChanged(bb::cascades::DataModelChangeType::Type, QSharedPointer<bb::cascades::DataModel::IndexMapper>)),
	        this, SLOT(itemsCountChangedAdapter(bb::cascades::DataModelChangeType::Type))); Q_ASSERT(res);
	Q_UNUSED(res);
}

PwGroup::~PwGroup() {
	clear();
}

void PwGroup::clear() {
    _uuid.clear();
    _iconId = 0;
    Util::safeClear(_name);
    Util::safeClear(_notes);
    sortedGroups.clear();
    qDeleteAll(_subGroups);
	_subGroups.clear();
	sortedEntries.clear();
	qDeleteAll(_entries);
	_entries.clear();
	emit itemsChanged(bb::cascades::DataModelChangeType::Init);

	QDateTime now = QDateTime::currentDateTime();
    _creationTime = now;
    _lastModificationTime = now;
    _lastAccessTime = now;
    _expiryTime = now;

    _isChildrenModified = true;
    _expires = false;
    _deleted = false;
    _parentGroup = NULL;
    _database = NULL;
}

void PwGroup::deleteWithoutBackup() {
    PwGroup* parentGroup = getParentGroup();
    if (parentGroup) {
        parentGroup->removeSubGroup(this);
    }
}

void PwGroup::addSubGroup(PwGroup* subGroup) {
    Q_ASSERT(subGroup);

	if (!subGroup->parent()) {
		subGroup->setParent(this);
	}
	subGroup->setParentGroup(this);

	// re-sort children when subgroup name changed
	bool res = QObject::connect(subGroup, SIGNAL(nameChanged(QString)), this, SLOT(sortChildren())); Q_ASSERT(res); Q_UNUSED(res);
	_subGroups.append(subGroup);
	emit itemsChanged(bb::cascades::DataModelChangeType::AddRemove);
	_isChildrenModified = true;
}

void PwGroup::removeSubGroup(PwGroup* subGroup) {
    Q_ASSERT(subGroup);
    if (this == subGroup->getParentGroup()) {
        _subGroups.removeOne(subGroup);
        subGroup->setParentGroup(NULL);
        bool res = QObject::disconnect(subGroup, SIGNAL(nameChanged(QString)), this, SLOT(sortChildren())); Q_ASSERT(res); Q_UNUSED(res);
        // but this group remains the entry's QObject parent, i.e. responsible for memory cleanup

        emit itemsChanged(bb::cascades::DataModelChangeType::AddRemove);
        _isChildrenModified = true;
    }
}

void PwGroup::addEntry(PwEntry* entry) {
    Q_ASSERT(entry);

    if (!entry->parent()) {
        entry->setParent(this);
    }
    entry->setParentGroup(this);

    // re-sort children when entry title changed
    bool res = QObject::connect(entry, SIGNAL(titleChanged(QString)), this, SLOT(sortChildren())); Q_ASSERT(res); Q_UNUSED(res);

    _entries.append(entry);
	emit itemsChanged(bb::cascades::DataModelChangeType::AddRemove);
	_isChildrenModified = true;
}

void PwGroup::removeEntry(PwEntry* entry) {
    Q_ASSERT(entry);
    if (this == entry->getParentGroup()) {
        _entries.removeOne(entry);
        entry->setParentGroup(NULL);
        bool res = QObject::disconnect(entry, SIGNAL(titleChanged(QString)), this, SLOT(sortChildren())); Q_ASSERT(res); Q_UNUSED(res);
        // but this group remains the entry's QObject parent, i.e. responsible for memory cleanup
        emit itemsChanged(bb::cascades::DataModelChangeType::AddRemove);
        _isChildrenModified = true;
    }
}

/**
 * Moves entry from its parent group to this one.
 */
void PwGroup::moveEntry(PwEntry* entry) {
    Q_ASSERT(entry);

    // Ok, we need to add the entry to this group and remove it from the original one,
    // while making sure that no signals are emitted while in intermediate state.

    PwGroup* originalParentGroup = entry->getParentGroup();

    entry->setParent(this);
    entry->setParentGroup(this);
    // re-sort children when entry title changed
    bool res = QObject::connect(entry, SIGNAL(titleChanged(QString)), this, SLOT(sortChildren())); Q_ASSERT(res); Q_UNUSED(res);
    _entries.append(entry);
    emit itemsChanged(bb::cascades::DataModelChangeType::AddRemove);
    _isChildrenModified = true;

    if (originalParentGroup) {
        originalParentGroup->_entries.removeOne(entry);
        res = QObject::disconnect(entry, SIGNAL(titleChanged(QString)), originalParentGroup, SLOT(sortChildren())); Q_ASSERT(res); Q_UNUSED(res);
        emit originalParentGroup->itemsChanged(bb::cascades::DataModelChangeType::AddRemove);
        originalParentGroup->_isChildrenModified = true;
    }
}

bool PwGroup::lessThan(const PwGroup* g1, const PwGroup* g2) {
    return g1->getName().compare(g2->getName(), Qt::CaseInsensitive) < 0;
}

void PwGroup::sortChildren() {
    sortedEntries = _entries;
    sortedGroups = _subGroups;
    if (Settings::instance()->isAlphaSorting()) {
        qStableSort(sortedGroups.begin(), sortedGroups.end(), PwGroup::lessThan);
        qStableSort(sortedEntries.begin(), sortedEntries.end(), PwEntry::lessThan);
    }
    emit itemsChanged(bb::cascades::DataModelChangeType::Update);
    _isChildrenModified = false;
}

/* DataModel interface implementation */
QVariant PwGroup::data(const QVariantList& indexPath) {
    if (_isChildrenModified) {
        sortChildren();
    }

    QVariant res;
    if (indexPath.length() == 0) {
        res = res.fromValue(this);
    } else if (indexPath.length() == 1) {
        int i = indexPath.first().toInt(NULL);
        if (i < sortedGroups.count()) {
            PwGroup* subgroup = sortedGroups.at(i);
            res = res.fromValue(subgroup);
        } else {
            PwEntry* entry = sortedEntries.at(i - sortedGroups.count());
            res = res.fromValue(entry);
        }
    } else {
        res = QVariant::Invalid;
    }
    return res;
}

int PwGroup::childCount(const QVariantList& indexPath) {
    if (_isChildrenModified) {
        sortChildren();
    }

    int result = 0;
    if (indexPath.length() == 0) {
        result = sortedGroups.count() + sortedEntries.count();
    }
    return result;
}
bool PwGroup::hasChildren(const QVariantList& indexPath) {
    if (_isChildrenModified) {
        sortChildren();
    }

    bool result = (indexPath.length() == 0) &&
            (!sortedGroups.isEmpty() || !sortedEntries.isEmpty());
    return result;
}
QString PwGroup::itemType(const QVariantList& indexPath) {
    if (_isChildrenModified) {
        sortChildren();
    }

    QString result;
    if (indexPath.length() == 0) {
        result = "group";
    } else if (indexPath.length() == 1) {
        int i = indexPath.first().toInt(NULL);
        result = (i < sortedGroups.count()) ? "group" : "entry";
    } else {
        result = "none";
    }
    return result;
}

void PwGroup::filterEntries(const SearchParams& params, QList<PwEntry*> &result) const {
    if (!params.includeDeleted && isDeleted())
        return;

    if (params.includeSubgroups) {
        PwGroup* subGroup;
        for (int i = 0; i < _subGroups.size(); i++) {
            subGroup = _subGroups.at(i);
            subGroup->filterEntries(params, result);
        }
    }

    PwEntry* entry;
    for (int i = 0; i < _entries.size(); i++) {
        entry = _entries.at(i);
        if (entry->matchesQuery(params.query)) {
            result.append(entry);
            // this group remains the parent of the entry
        }
    }
}

/**
 * Searches all subgroups recursively, returns the (first) one with the given UUID,
 * or NULL if none found;
 */
PwGroup* PwGroup::findGroupByUuid(const PwUuid& uuid) const {
    if (this->_uuid == uuid)
        return const_cast<PwGroup*>(this);

    PwGroup* result = NULL;
    for (int i = 0; i < _subGroups.size(); i++) {
        result = _subGroups.at(i)->findGroupByUuid(uuid);
        if (result)
            break;
    }
    return result;
}

QString PwGroup::toString() const {
    return "PwGroup[" + _name + "]";
}

/**
 * Returns true if the group has expired.
 */
bool PwGroup::isExpired() const {
    return isExpires() && (QDateTime::currentDateTime() > getExpiryTime());
}

/** Updates last access timestamp to current time */
void PwGroup::registerAccessEvent() {
    QDateTime now = QDateTime::currentDateTime();
    setLastAccessTime(now);
}
/** Updates modification timestamp to current time */
void PwGroup::registerModificationEvent() {
    registerAccessEvent();

    QDateTime now = QDateTime::currentDateTime();
    setLastModificationTime(now);
}


// matches signatures of the itemsChanged() signal with the itemsCountChanged()
void PwGroup::itemsCountChangedAdapter(DataModelChangeType::Type changeType) {
    if (changeType != DataModelChangeType::Update) {
        emit itemsCountChanged(immediateChildCount());
    }
}

void PwGroup::setUuid(const PwUuid& uuid) {
    if (uuid != _uuid) {
        _uuid = uuid;
        emit uuidChanged(uuid);
    }
}
void PwGroup::setIconId(int iconId) {
    if (iconId != _iconId) {
        _iconId = iconId;
        emit iconIdChanged(iconId);
    }
}

void PwGroup::setName(const QString& name) {
    if (name != _name) {
        _name = Util::deepCopy(name);
        emit nameChanged(_name);
    }
}

void PwGroup::setNotes(const QString& notes) {
    if (notes != _notes) {
        _notes = Util::deepCopy(notes);
        emit notesChanged(_notes);
    }
}

void PwGroup::setCreationTime(const QDateTime& time) {
    if (time != _creationTime) {
        _creationTime = time;
        emit creationTimeChanged(time);
    }
}

void PwGroup::setLastModificationTime(const QDateTime& time) {
    if (time != _lastModificationTime) {
        _lastModificationTime = time;
        emit lastModificationTimeChanged(time);
    }
}

void PwGroup::setLastAccessTime(const QDateTime& time) {
    if (time != _lastAccessTime) {
        _lastAccessTime = time;
        emit lastAccessTimeChanged(time);
    }
}

void PwGroup::setExpiryTime(const QDateTime& time) {
    if (time != _expiryTime) {
        _expiryTime = time;
        emit expiryTimeChanged(time);
    }
}

void PwGroup::setExpires(bool expires) {
    if (expires != _expires) {
        _expires = expires;
        emit expiresChanged(expires);
    }
}

void PwGroup::setDeleted(bool deleted) {
    if (deleted != _deleted) {
        _deleted = deleted;
        emit deletedChanged(deleted);
    }
}

void PwGroup::setParentGroup(PwGroup* parentGroup) {
    if (parentGroup != _parentGroup) {
        _parentGroup = parentGroup;
        emit parentGroupChanged(parentGroup);
    }
}

void PwGroup::setDatabase(PwDatabase* database) {
    if (database != _database) {
        _database = database;
        emit databaseChanged(database);
    }
}

/**
 * Recursively iterates through all the children groups and entries of this group
 * and adds them to the given lists. The group itself is excluded.
 */
void PwGroup::getAllChildren(QList<PwGroup*> &childGroups, QList<PwEntry*> &childEntries) const {
    QList<PwGroup*> groups = this->getSubGroups();
    for (int i = 0; i < groups.size(); i++) {
        PwGroup* gr = groups.at(i);
        childGroups.append(gr);
        gr->getAllChildren(childGroups, childEntries);
    }
    QList<PwEntry*> entries = this->getEntries();
    for (int i = 0; i < entries.size(); i++) {
        childEntries.append(entries.at(i));
    }
}
