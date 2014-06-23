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

PwGroup::PwGroup() : bb::cascades::DataModel(), _uuid(),
        _creationTime(), _lastModificationTime(),
        _lastAccessTime(), _expiryTime() {
	_name = QString::null;
	_notes = QString::null;
	_iconId = 0;
	_isChildrenModified = false;
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
	qDeleteAll(_subGroups);
	_subGroups.clear();
	qDeleteAll(_entries);
	_entries.clear();
    _creationTime.setMSecsSinceEpoch(0L);
    _lastModificationTime.setMSecsSinceEpoch(0L);
    _lastAccessTime.setMSecsSinceEpoch(0L);
    _expiryTime.setMSecsSinceEpoch(0L);
    _isChildrenModified = true;
}

void PwGroup::addSubGroup(PwGroup* subGroup) {
	if (!subGroup->parent()) {
		subGroup->setParent(this);
	}
	_subGroups.append(subGroup);
	_isChildrenModified = true;
}

void PwGroup::addEntry(PwEntry* entry) {
    if (!entry->parent()) {
        entry->setParent(this);
    }
	_entries.append(entry);
	_isChildrenModified = true;
}

bool PwGroup::lessThan(const PwGroup* g1, const PwGroup* g2) {
    return g1->getName().compare(g2->getName(), Qt::CaseInsensitive) < 0;
}

void PwGroup::sortChildren() {
    qStableSort(_subGroups.begin(), _subGroups.end(), PwGroup::lessThan);
    qStableSort(_entries.begin(), _entries.end(), PwEntry::lessThan);
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
        if (i < _subGroups.count()) {
            PwGroup* subgroup = _subGroups.at(i);
            res = res.fromValue(subgroup);
        } else {
            PwEntry* entry = _entries.at(i - _subGroups.count());
            res = res.fromValue(entry);
        }
    } else {
        res = QVariant::Invalid;
    }
    return res;
}

int PwGroup::childCount(const QVariantList& indexPath) {
    int result = 0;
    if (indexPath.length() == 0) {
        result = _subGroups.count() + _entries.count();
    }
    return result;
}
bool PwGroup::hasChildren(const QVariantList& indexPath) {
    bool result = (indexPath.length() == 0) &&
            (!_subGroups.isEmpty() || !_entries.isEmpty());
    return result;
}
QString PwGroup::itemType(const QVariantList& indexPath) {
    QString result;
    if (indexPath.length() == 0) {
        result = "group";
    } else if (indexPath.length() == 1) {
        int i = indexPath.first().toInt(NULL);
        result = (i < _subGroups.count()) ? "group" : "entry";
    } else {
        result = "none";
    }
    return result;
}

void PwGroup::filterEntries(const QString& query, QList<PwEntry*> &result, bool includeSubgroups) const {
    if (includeSubgroups) {
        PwGroup* subGroup;
        for (int i = 0; i < _subGroups.size(); i++) {
            subGroup = _subGroups.at(i);
            subGroup->filterEntries(query, result, includeSubgroups);
        }
    }

    PwEntry* entry;
    for (int i = 0; i < _entries.size(); i++) {
        entry = _entries.at(i);
        if (entry->matchesQuery(query)) {
            result.append(entry);
            // this group remains the parent of the entry
        }
    }
}

QString PwGroup::toString() const {
    return "PwGroup[" + _name + "]";
}
