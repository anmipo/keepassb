/*
 * PwGroupV4.h
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#ifndef PWGROUPV4_H_
#define PWGROUPV4_H_

#include "db/PwGroup.h"
#include <QtXml/QXmlStreamReader>
#include "db/v4/DefsV4.h"
#include "db/v4/PwMetaV4.h"
#include "crypto/CryptoManager.h"

class PwGroupV4: public PwGroup {
    Q_OBJECT
private:
    /** Reads group's timestamps from MXL */
    ErrorCodesV4::ErrorCode readTimes(QXmlStreamReader& xml);

public:
    PwGroupV4(QObject* parent=0);
    virtual ~PwGroupV4();

    /**
     * Moves the group's whole branch to Backup group.
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
     * Loads group fields from the stream.
     * The caller is responsible for clearing any previous values.
     */
    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml, PwDatabaseV4Meta& meta, Salsa20& salsa20);
};

Q_DECLARE_METATYPE(PwGroupV4*);

#endif /* PWGROUPV4_H_ */
