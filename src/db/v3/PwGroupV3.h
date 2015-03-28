/*
 * PwGroupV3.h
 *
 *  Created on: 19 Aug 2014
 *      Author: Andrei
 */

#ifndef PWGROUPV3_H_
#define PWGROUPV3_H_

#include <QObject>
#include "db/PwGroup.h"

class PwEntryV3;

class PwGroupV3: public PwGroup {
    Q_OBJECT
private:
    qint32 _id;
    quint16 _level;
    qint32 _flags;   // some internal KeePass field

    const static quint16 FIELD_RESERVED  = 0x0000;
    const static quint16 FIELD_GROUP_ID  = 0x0001;
    const static quint16 FIELD_NAME      = 0x0002;
    const static quint16 FIELD_CREATION_TIME      = 0x0003;
    const static quint16 FIELD_LAST_MODIFIED_TIME = 0x0004;
    const static quint16 FIELD_LAST_ACCESS_TIME   = 0x0005;
    const static quint16 FIELD_EXPIRATION_TIME    = 0x0006;
    const static quint16 FIELD_ICON_ID            = 0x0007;
    const static quint16 FIELD_GROUP_LEVEL        = 0x0008;
    const static quint16 FIELD_GROUP_FLAGS        = 0x0009;
    const static quint16 FIELD_END                = 0xFFFF;

public:
    // Fixed values of the Backup/RecycleBin group
    const static QString BACKUP_GROUP_NAME;
    const static int BACKUP_GROUP_ICON_ID = 4;

    PwGroupV3(QObject* parent=0);
    virtual ~PwGroupV3();

    // add/remove group/entry are overriden to also handle V3-specific properties: groupID and level
    virtual void addSubGroup(PwGroup* subGroup);
    virtual void removeSubGroup(PwGroup* subGroup);
    virtual void addEntry(PwEntry* entry);
    virtual void removeEntry(PwEntry* entry);
    virtual void moveEntry(PwEntry* entry);

    /**
     * Moves all the group's entries (recursively) to Backup group; subgroups are deleted.
     * Returns true if successful.
     */
    virtual bool moveToBackup();

    /**
     * Creates an entry in the group and returns a reference to it.
     */
    virtual PwEntry* createEntry();
    /**
     * Creates a subgroup in the group and returns a reference to it.
     */
    virtual PwGroup* createGroup();

    /**
     * Checks if a group name is reserved for internal use and cannot be assigned by the user.
     */
    virtual bool isNameReserved(const QString& name);

    /** Loads group fields from the stream. Returns true on success, false in case of error. */
    bool readFromStream(QDataStream& stream);
    /** Writes group fields to the stream. Returns true on success, false in case of error. */
    bool writeToStream(QDataStream& stream);

    virtual void clear();

    // property accessors
    void setId(const qint32 id) { this->_id = id; }
    qint32 getId() const { return _id; }
    void setLevel(const quint16 level) { this->_level = level; }
    quint16 getLevel() const { return _level; }
    void setFlags(const qint32 flags) { this->_flags = flags; }
    qint32 getFlags() const { return _flags; }
};

#endif /* PWGROUPV3_H_ */
