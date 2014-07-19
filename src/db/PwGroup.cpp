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
#include "util/Settings.h"

PwGroup::PwGroup() : bb::cascades::DataModel(), _uuid(),
        _creationTime(), _lastModificationTime(),
        _lastAccessTime(), _expiryTime(),
        _subGroups(), sortedGroups(),
        _entries(), sortedEntries() {
	_name = QString::null;
	_notes = QString::null;
	_iconId = 0;
	_isChildrenModified = false;
	_deleted = false;
	_parentGroup = NULL;

	bool res = QObject::connect(Settings::instance(), SIGNAL(alphaSortingChanged(bool)), this, SLOT(sortChildren()));
	Q_ASSERT(res);
}

PwGroup::~PwGroup() {
	qDebug() << "deleting group: " << _name;
	clear();
}

void PwGroup::clear() {
    _uuid.clear();
    _iconId = 0;
    _name.clear();
    _notes.clear();
    sortedGroups.clear();
	qDeleteAll(_subGroups);
	_subGroups.clear();
	sortedEntries.clear();
	qDeleteAll(_entries);
	_entries.clear();
    _creationTime.setMSecsSinceEpoch(0L);
    _lastModificationTime.setMSecsSinceEpoch(0L);
    _lastAccessTime.setMSecsSinceEpoch(0L);
    _expiryTime.setMSecsSinceEpoch(0L);
    _isChildrenModified = true;
    _deleted = false;
}

void PwGroup::addSubGroup(PwGroup* subGroup) {
    Q_ASSERT(subGroup);

	if (!subGroup->parent()) {
		subGroup->setParent(this);
	}
	subGroup->setParentGroup(this);
	_subGroups.append(subGroup);
	_isChildrenModified = true;
}

void PwGroup::addEntry(PwEntry* entry) {
    Q_ASSERT(entry);

    if (!entry->parent()) {
        entry->setParent(this);
    }
    entry->setParentGroup(this);
	_entries.append(entry);
	_isChildrenModified = true;
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
    emit itemsChanged(bb::cascades::DataModelChangeType::Update, QSharedPointer<IndexMapper>());
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

QString PwGroup::toString() const {
    return "PwGroup[" + _name + "]";
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
        _name = name;
        emit nameChanged(name);
    }
}

void PwGroup::setNotes(const QString& notes) {
    if (notes != _notes) {
        _notes = notes;
        emit notesChanged(notes);
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
