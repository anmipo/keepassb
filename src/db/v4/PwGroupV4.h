/*
 * PwGroupV4.h
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#ifndef PWGROUPV4_H_
#define PWGROUPV4_H_

#include "db/PwGroup.h"
#include <QtXml/QXmlStreamReader>
#include "db/v4/DefsV4.h"
#include "db/v4/PwMetaV4.h"
#include "crypto/CryptoManager.h"
#include "util/ProgressObserver.h"

class PwGroupV4: public PwGroup {
    Q_OBJECT
    Q_PROPERTY(bool isExpanded READ getIsExpanded WRITE setIsExpanded NOTIFY expandedChanged)
    Q_PROPERTY(PwUuid customIconUuid READ getCustomIconUuid WRITE setCustomIconUuid NOTIFY customIconUuidChanged)
    Q_PROPERTY(QString defaultAutoTypeSequence READ getDefaultAutoTypeSequence WRITE setDefaultAutoTypeSequence NOTIFY defaultAutoTypeSequenceChanged)
    Q_PROPERTY(QString enableAutoType READ getEnableAutoType WRITE setEnableAutoType NOTIFY enableAutoTypeChanged)
    Q_PROPERTY(QString enableSearching READ getEnableSearching WRITE setEnableSearching NOTIFY enableSearchingChanged)
private:
    bool _isExpanded;
    PwUuid _customIconUuid;
    QString _defaultAutoTypeSequence;
    QString _enableAutoType; // actually a bool, but with possible "null" value
    QString _enableSearching; // actually a bool, but with possible "null" value
    PwUuid _lastTopVisibleEntryUuid;
    quint32 _usageCount;
    QDateTime _locationChangedTime;

    /** Reads group's timestamps from MXL */
    ErrorCodesV4::ErrorCode readTimes(QXmlStreamReader& xml);

public:
    PwGroupV4(QObject* parent=0);
    virtual ~PwGroupV4();

    virtual void clear();

    /** Updates last access timestamp to current time and changes usage counter */
    virtual void registerAccessEvent();

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
     */
    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml, PwMetaV4& meta, Salsa20& salsa20, ProgressObserver* progressObserver);

    /**
     * Writes the group with all its entries, subgroups and their subentries to the stream.
     */
    void writeToStream(QXmlStreamWriter& xml, PwMetaV4& meta, Salsa20& salsa20, ProgressObserver* progressObserver);

    // property getters/setters
    bool getIsExpanded() const { return _isExpanded; }
    void setIsExpanded(bool expanded);
    PwUuid getCustomIconUuid() const { return _customIconUuid; }
    void setCustomIconUuid(const PwUuid& uuid);
    QString getDefaultAutoTypeSequence() const { return _defaultAutoTypeSequence; }
    void setDefaultAutoTypeSequence(const QString& defaultAutoTypeSequence);
    QString getEnableAutoType() const { return _enableAutoType; }
    void setEnableAutoType(const QString& enableAutoType);
    QString getEnableSearching() const { return _enableSearching; }
    void setEnableSearching(const QString& enableSearching);
    PwUuid getLastTopVisibleEntry() const { return _lastTopVisibleEntryUuid; }
    void setLastTopVisibleEntry(const PwUuid& uuid);
    quint32 getUsageCount() const { return _usageCount; }
    void setUsageCount(const quint32 usageCount);
    QDateTime getLocationChangedTime() const { return _locationChangedTime; }
    void setLocationChangedTime(const QDateTime& locationChangedTime);

signals:
    void expandedChanged(bool);
    void customIconUuidChanged(PwUuid);
    void defaultAutoTypeSequenceChanged(QString);
    void enableAutoTypeChanged(QString);
    void enableSearchingChanged(QString);
    void lastTopVisibleEntryChanged(PwUuid);
    void usageCountChanged(quint32);
    void locationChangedTimeChanged(QDateTime);
};

Q_DECLARE_METATYPE(PwGroupV4*);

#endif /* PWGROUPV4_H_ */
