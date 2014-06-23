/*
 * PwEntryV4.h
 *
 *  Created on: 3 Jun 2014
 *      Author: andrei.popleteev
 */

#ifndef PWENTRYV4_H_
#define PWENTRYV4_H_

#include "db/PwEntry.h"
#include <QMetaType>
#include <QList>
#include <bb/cascades/DataModel>
#include <bb/cascades/QListDataModel>

class PwExtraField: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(QString value READ getValue NOTIFY valueChanged)
private:
    QString _name;
    QString _value;
public:
    PwExtraField();
    PwExtraField(const QString& name, const QString& value);
    virtual ~PwExtraField() {}

    // property accessors
    QString getName() const { return _name; }
    QString getValue() const { return _value; }
signals:
    void nameChanged(QString name);
    void valueChanged(QString value);
};

class PwEntryV4: public PwEntry {
    Q_OBJECT
//    Q_PROPERTY(DataModel extraFieldsDataModel READ getExtraFieldsDataModel NOTIFY extraFieldsDataModelChanged);
//    Q_PROPERTY(DataModel historyDataModel READ getHistoryDataModel NOTIFY historyDataModelChanged);
    Q_PROPERTY(int extraSize READ getExtraSize NOTIFY extraSizeChanged)
    Q_PROPERTY(int historySize READ getHistorySize NOTIFY historySizeChanged)
private:
    QMap<QString, QString> fields;
    bb::cascades::QListDataModel<PwExtraField*> _extraFieldsDataModel;
    bb::cascades::QListDataModel<PwEntryV4*> _historyDataModel;

    bool isStandardField(const QString& name) const;

public:
    PwEntryV4();
    virtual ~PwEntryV4();

    virtual void clear();

    void addHistoryEntry(PwEntryV4* historyEntry);

    /** Search helper. Returns true if any of the fields contain the query string. */
    virtual bool matchesQuery(const QString& query) const;

    /**
     * Adds a named field value to the entry.
     */
    void setField(const QString& name, const QString& value);

    Q_INVOKABLE bb::cascades::DataModel* getExtraFieldsDataModel() { return &_extraFieldsDataModel; }
    Q_INVOKABLE bb::cascades::DataModel* getHistoryDataModel() { return &_historyDataModel; }

    // property accessors
    virtual QString getTitle() const;
    virtual void setTitle(const QString& title);
    virtual QString getUserName() const;
    virtual void setUserName(const QString& userName);
    virtual QString getPassword() const;
    virtual void setPassword(const QString& password);
    virtual QString getUrl() const;
    virtual void setUrl(const QString& url);
    virtual QString getNotes() const;
    virtual void setNotes(const QString& notes);
    int getHistorySize() { return _historyDataModel.size(); }
    int getExtraSize() { return _extraFieldsDataModel.size(); }

signals:
    void extraFieldsDataModelChanged();
    void historyDataModelChanged();
    void historySizeChanged();
    void extraSizeChanged();
};

Q_DECLARE_METATYPE(PwExtraField*);
Q_DECLARE_METATYPE(PwEntryV4*);

#endif /* PWENTRYV4_H_ */
