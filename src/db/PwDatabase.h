/*
 * PwDatabase.h
 *
 *  Created on: 26 May 2014
 *      Author: andrei.popleteev
 */

#ifndef PWDATABASE_H_
#define PWDATABASE_H_

#include <QObject>
#include <bb/cascades/QListDataModel>
#include "db/PwGroup.h"
#include "db/PwEntry.h"

class PwDatabaseFacade;

class SearchParams;

class PwDatabase : public QObject {
    Q_OBJECT
protected:
	PwGroup* _rootGroup;

	// returns true if successful. If data is invalid/empty, returns an empty key
	virtual bool processKeyFile(const QByteArray& keyFileData, QByteArray& key) const;
public:
    enum Error {
        SUCCESS           = 0,
        UNKNOWN_DB_FORMAT = 1
        // child classes' specific codes start from 0x10
    };
	PwDatabase(QObject* parent=0);
	virtual ~PwDatabase();

    /**
     * Tries to decrypt the given DB data with given password/key.
     * The progress and the result are communicated asynchronously via signals.
     */
    virtual void unlock(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) = 0;
    // Clears the database.
    virtual void clear();
    // Clears and locks the database.
    virtual void lock();

    /**
     * Returns the entries which contain given query text
     */
    virtual int search(const SearchParams& params, QList<PwEntry*> &searchResult) const;

    PwGroup* getRootGroup();
signals:
    /**
     * Emitted once the DB has been locked.
     */
    void dbLocked();
    /**
     * Emitted once the DB has been successfully unlocked.
     */
    void dbUnlocked();
    /**
     * Periodically emitted during the unlocking process.
     */
    void unlockProgressChanged(const int progressPercent);
    /**
     * Emitted in case of generic DB loading/decryption error.
     * errorCode parameter is one of PwDatabaseV4::Errors values.
     */
    void dbUnlockError(const QString& message, const int errorCode);
    /**
     * Emitted when password/key seem to be invalid (decrypted checksum mismatch).
     */
    void invalidPasswordOrKey();
};

/**********************************/

/**
 * Data model for search results
 */
class PwSearchResultDataModel: public bb::cascades::QListDataModel<PwEntry*> {
    Q_OBJECT
public:
    PwSearchResultDataModel(QObject* parent=0);
    virtual ~PwSearchResultDataModel() {}
};

/*********************************/
class PwDatabaseFacade: public QObject {
    Q_OBJECT
    Q_PROPERTY(PwGroup* rootGroup READ getRootGroup)
    Q_PROPERTY(PwSearchResultDataModel* searchResult READ getSearchResult NOTIFY searchResultChanged)
    Q_PROPERTY(bool locked READ isLocked NOTIFY lockedChanged);
private:
    bool _locked;
    PwDatabase* db;
    PwSearchResultDataModel _searchResultDataModel;

    // Creates a PwDatabase instance suitable for processing the given data.
    // If no suitable processor found, returns NULL.
    static PwDatabase* createDatabaseInstance(const QByteArray& rawDbData);

    // Registers DB-related types for use in QML
    void registerQmlTypes();

    // Clears the current DB instance and search results, if any.
    void clear();

    void setLocked(bool locked);
private slots:
    // called whenever the contained DB locks/unlocks
    void onDbUnlocked();
    void onDbLocked();
public:
    PwDatabaseFacade(QObject* parent=0);
    virtual ~PwDatabaseFacade();


    /**
     * Initiates DB unlocking.
     * The progress and the result is communicated via appropriate signal.
     */
    Q_INVOKABLE void unlock(const QString &dbFilePath, const QString &password, const QString &keyFilePath);

    // Property getters/setters
    bool isLocked() const { return _locked; }
    PwGroup* getRootGroup() const { return (db ? db->getRootGroup() : NULL); }
    PwSearchResultDataModel* getSearchResult() { return &_searchResultDataModel; }

public slots:
    /**
     * Locks/closes the current DB instance, if any.
     */
    Q_INVOKABLE void lock();
    /**
     * Finds entries which contain given query text, and publishes them via searchResult property.
     * Returns the number of found entries.
     */
    Q_INVOKABLE int search(const QString& query);

signals:
    void dbLocked();
    void dbUnlocked();
    void dbUnlockError(const QString& message, const int errorCode);
    void fileOpenError(const QString& message);
    void invalidPasswordOrKey();
    void unlockProgressChanged(const int progressPercent);
    void searchResultChanged();
    void lockedChanged(bool);
};

#endif /* PWDATABASE_H_ */
