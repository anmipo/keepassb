/*
 * PwDatabase.h
 *
 *  Created on: 26 May 2014
 *      Author: andrei.popleteev
 */

#ifndef PWDATABASE_H_
#define PWDATABASE_H_

#include <QObject>
#include <bb/cascades/GroupDataModel>
#include "db/PwGroup.h"
#include "db/PwEntry.h"
#include "db/PwIcon.h"

class PwDatabaseFacade;

class SearchParams;

class PwDatabase : public QObject, public ProgressObserver {
    Q_OBJECT
public:
    enum ErrorCode {
        SUCCESS             = 0,
        UNKNOWN_DB_FORMAT   = 1,
        COMPOSITE_KEY_ERROR = 2,
        DB_FILE_EMPTY       = 3,
        KEY_COMPOSING_ERROR = 4,
        KEY_TRANSFORM_INIT_ERROR = 0x05,
        KEY_TRANSFORM_ERROR_1    = 0x06,
        KEY_TRANSFORM_ERROR_2    = 0x07,
        KEY_TRANSFORM_ERROR_3    = 0x08,

        // child classes' specific codes start from 0x10
    };

protected:
	PwGroup* _rootGroup;
	QString _dbFilePath;

	// returns true if successful. If data is invalid/empty, returns an empty key
	virtual bool processKeyFile(const QByteArray& keyFileData, QByteArray& key) const;

    /**
     * Extracts the key from a correctly-formed XML file.
     * Returns true if successful, false otherwise.
     */
    virtual bool processXmlKeyFile(const QByteArray& keyFileData, QByteArray& key) const = 0;

    /** Combines password and key data into one key */
	virtual bool buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const = 0;

	/** Returns the number of all groups and entries combined */
	int countAllChildren() const;

	/** Converts the password string to its raw representation, as of format version's rules */
	virtual QByteArray getPasswordBytes(const QString& password) const = 0;

    /** Setter for the combinedKey field (in child classes) */
	virtual void setCombinedKey(const QByteArray& newKey) = 0;

    // Calculates the AES encryption key based on the combined key (password + key data)
    // and current header seed values.
	ErrorCode transformKey(const QByteArray& masterSeed, const QByteArray& transformSeed,
	        quint64 transformRounds, const QByteArray& combinedKey, QByteArray& aesKey);
    // Helper function for multithreaded key transformation
    ErrorCode performKeyTransformRounds(const unsigned char* pTransformSeed,
            unsigned char* pTransKey, const quint64 nRounds, bool reportProgress);

    /**
     * Callback for progress updates of time-consuming processes.
     */
    void onProgress(quint8 progressPercent);

public:
	PwDatabase(QObject* parent=0);
	virtual ~PwDatabase();

    /**
     * Tries to decrypt and load the given DB data with given password/key.
     * The progress and the result are communicated asynchronously via signals.
     */
    virtual void load(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData) = 0;
    /**
     * Encrypts the DB and stores its content to the given array.
     * In case of error emits a dbSaveError with error code and returns false.
     */
    virtual bool save(QByteArray& outData) = 0;

    /**
     * Changes DB's master key to the given combination.
     * Returns true if successful, otherwise emits an error and returns false.
     */
    virtual bool changeMasterKey(const QString& password, const QByteArray& keyFileData);

    // Clears the database.
    virtual void clear();
    // Clears and locks the database.
    virtual void lock();
    /** Returns the database format version */
    virtual int getFormatVersion() = 0;

    /**
     * Returns the entries which contain given query text
     */
    virtual int search(const SearchParams& params, QList<PwEntry*> &searchResult) const;

    PwGroup* getRootGroup() const;

    /**
     * Returns the Backup group of this database.
     * If createIfMissing is true, creates the group if it is missing.
     * (However, if backup is disabled (e.g. in V4) will not create anything and still return NULL).
     */
    virtual PwGroup* getBackupGroup(bool createIfMissing = false) = 0;

    /** Stores DB file path */
    void setDatabaseFilePath(const QString& dbFilePath);
    /** Returns full path to the DB file */
    QString getDatabaseFilePath() const;
    /** Returns only file name of the DB file (file.ext) */
    QString getDatabaseFileName() const;
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
     * Periodically emitted during the decryption/encryption process.
     */
    void progressChanged(const int progressPercent);
    /**
     * Emitted in case of generic DB loading/decryption error.
     * errorCode parameter is one of PwDatabase::Error/PwDatabaseV3::Error/PwDatabaseV4::Error values.
     */
    void dbLoadError(const QString& message, const int errorCode);
    /**
     * Emitted in case of DB saving error.
     * errorCode parameter is one of PwDatabase::Error/PwDatabaseV3::Error/PwDatabaseV4::Error values.
     */
    void dbSaveError(const QString& message, const int errorCode);
    /**
     * Emitted when password/key seem to be invalid (decrypted checksum mismatch).
     */
    void invalidPasswordOrKey();
    /**
     * Emitted when database file path/name changes
     */
    void dbFileNameChanged(QString dbFileName);
};

/**********************************/

/**
 * Data model for search results
 */
class PwSearchResultDataModel: public bb::cascades::GroupDataModel {
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
    /**
     * Loads given key file to the given buffer. No processing, just load or emit error signals.
     * Empty file path is ok.
     * Returns true if successful, otherwise emits fileOpenError signal and returns false.
     */
    bool loadKeyFile(const QString& keyFilePath, QByteArray& keyFileData) const;

    // Registers DB-related types for use in QML
    void registerQmlTypes();

    /** Establishes signal forwarding from DB instance to this facade */
    void connectDatabaseSignals();

    // Clears the current DB instance and search results, if any.
    void clear();
    /**
     * Releases memory occupied by current DB instance (if any open),
     * optionally locking it first.
     */
    void releaseDatabase(bool lockFirst);

    /** Returns a timestamped backup file path for the given DB file path. */
    static QString makeBackupFilePath(QString dbFilePath);

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

    /**
     * Creates a sample v4 database with the given path and opens it.
     * THe DB only exists in memory, no file is written.
     * Master key is not initialized, so call changeMasterKey() afterwards.
     * Returns true if successful; false in case of any error.
     */
    Q_INVOKABLE bool createDatabaseV4(const QString& dbFilePath);

    /**
     * Changes the master key of the currently opened DB to the specified one and saves the DB.
     * Returns true if successful; otherwise emits a fileOpenError or dbSaveError and returns false.
     */
    Q_INVOKABLE bool changeMasterKey(const QString& password, const QString& keyFilePath);

    /** Returns the format version of the currently opened DB (3 or 4, or -1 if none opened) */
    Q_INVOKABLE int getFormatVersion() const { return (db ? db->getFormatVersion() : -1); }

    /** Returns full file path of the currently opened DB. */
    Q_INVOKABLE QString getDatabaseFilePath() const;

    // Property getters/setters
    bool isLocked() const { return _locked; }

    /**
     * Returns true if the current DB is a demo DB from app assets.
     * Returns false otherwise (real DB opened or there is no opened DB).
     */
    Q_INVOKABLE bool isDemoDatabase() const;

    /**
     * Indicates whether the DB can be edited/saved.
     */
    Q_INVOKABLE bool isEditable() const;

    /**
     * Returns true when the parameter is the DB root group; false otherwise.
     */
    Q_INVOKABLE bool isRoot(PwGroup* group) const;

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

    /**
     * Saves changes in the current database.
     * If successful, returns true and emits dbSaved() signal;
     * otherwise returns false and (sometimes) emits an appropriate error signal.
     */
    Q_INVOKABLE bool save();

signals:
    void dbLocked();
    void dbUnlocked();
    void dbUnlockError(const QString& message, const int errorCode);
    void fileOpenError(const QString& message, const QString& errorDescription) const;

    /** Emitted before starting DB encryption/saving */
    void dbAboutToSave();
    /** Emitted when DB is successfully saved */
    void dbSaved();
    /**
     * Emitted in case of DB encryption error.
     * errorCode parameter is one of PwDatabase::Error/PwDatabaseV3::Error/PwDatabaseV4::Error values.
     */
    void dbSaveError(const QString& message, const int errorCode);
    /**
     * Emitted in case of I/O error during DB saving.
     */
    void fileSaveError(const QString& message, const QString& errorDescription);

    void invalidPasswordOrKey();
    void progressChanged(const int progressPercent);
    void searchResultChanged();
    void lockedChanged(bool locked);
    void dbVersionChanged(int version);
};

#endif /* PWDATABASE_H_ */
