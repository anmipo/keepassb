/*
 * PwDatabase.cpp
 *
 *  Created on: 26 May 2014
 *      Author: andrei.popleteev
 */

#include "PwDatabase.h"
#include <QtXml/QXmlStreamReader>
#include "crypto/CryptoManager.h"
#include "db/v3/PwDatabaseV3.h"
#include "db/v4/PwDatabaseV4.h"
#include "db/PwGroup.h"
#include "db/PwEntry.h"
#include "util/Settings.h"
#include "util/Util.h"
#include "sbdef.h"

// String added to temporary DB file name when saving
const QString TMP_SAVE_FILE_NAME_SUFFIX = ".tmp";

const QString DEMO_DATABASE_FILE_PATH = "app/native/assets/demo.kdbx";


PwDatabase::PwDatabase(QObject* parent) : QObject(parent), _dbFilePath("") {
	_rootGroup = NULL;
}

PwDatabase::~PwDatabase() {
	LOG("deleting PwDatabase");
    clear();
}

void PwDatabase::lock() {
    this->clear();
    emit dbLocked();
	LOG("DB locked");
}

void PwDatabase::clear() {
    if (_rootGroup) {
        _rootGroup->clear();
        delete _rootGroup;
        _rootGroup = NULL;
    }
    Util::safeClear(_dbFilePath);
	LOG("DB cleared");
}

/** Returns the number of all groups and entries combined */
int PwDatabase::countAllChildren() const {
    int result = 0;
    PwGroup* root = getRootGroup();
    if (root) {
        // TODO can make this more efficient
        QList<PwGroup*> groups;
        QList<PwEntry*> entries;
        root->getAllChildren(groups, entries);
        result = groups.size() + entries.size();
    }
    return result;
}

bool PwDatabase::processKeyFile(const QByteArray& keyFileData, QByteArray& key) const {
    const int KEY_LENGTH = 32;

    key.clear();
    // Is it a key file?
    if (keyFileData.length() == KEY_LENGTH) {
        // assume it is a 32-byte key
        //key.setRawData(result.constData(), result.length());
        key = keyFileData;
        return true;
    } else if (keyFileData.length() == 2 * KEY_LENGTH) {
        // maybe a 64-byte hex encoded key?
        key = QByteArray::fromHex(keyFileData);
        if (key.length() == KEY_LENGTH) {
            //key.setRawData(result.constData(), result.length());
            return true;
        }
    }

    // Is it an XML key file?
    if (processXmlKeyFile(keyFileData, key) && (key.length() == KEY_LENGTH)) {
        return true;
    }

    // It is something else, just hash it
    if (CryptoManager::instance()->sha256(keyFileData, key) != SB_SUCCESS) {
        LOG("could not hash the key file data");
        return false;
    }
    return true;
}

PwGroup* PwDatabase::getRootGroup() const {
    return _rootGroup;
}

void PwDatabase::setDatabaseFilePath(const QString& dbFilePath) {
    if (dbFilePath != this->_dbFilePath) {
        this->_dbFilePath = dbFilePath;
        emit dbFileNameChanged(getDatabaseFileName());
    }
}

/** Returns full path to the DB file */
QString PwDatabase::getDatabaseFilePath() const {
    return _dbFilePath;
}

/** Returns only file name of the DB file (file.ext) */
QString PwDatabase::getDatabaseFileName() const {
    int sepPos = _dbFilePath.lastIndexOf("/");
    return _dbFilePath.mid(sepPos + 1);
}

int PwDatabase::search(const SearchParams& params, QList<PwEntry*> &searchResult) const {
    Q_ASSERT(_rootGroup != NULL);

    searchResult.clear();
    _rootGroup->filterEntries(params, searchResult);
    return searchResult.size();
}

/**
 * Changes DB's master key to the given combination.
 * Returns true if successful, otherwise emits an error and returns false.
 */
bool PwDatabase::changeMasterKey(const QString& password, const QByteArray& keyFileData) {
    QByteArray tmpCombinedKey;
    if (!buildCompositeKey(getPasswordBytes(password), keyFileData, tmpCombinedKey)) {
        emit dbSaveError(tr("Cryptographic library error", "Generic error message from a cryptographic library"), KEY_COMPOSING_ERROR);
        return false;
    }
    setCombinedKey(tmpCombinedKey);
    Util::safeClear(tmpCombinedKey);

    // transformKey() is part of the subsequent save() call
    return true;
}

/*****************************************/
PwSearchResultDataModel::PwSearchResultDataModel(QObject* parent) :
        bb::cascades::GroupDataModel(QStringList() << "groupPath", parent) {
    setGrouping(ItemGrouping::ByFullValue);
}
/*****************************************/

PwDatabaseFacade::PwDatabaseFacade(QObject* parent) : QObject(parent), _searchResultDataModel() {
    db = NULL;
    _locked = true;
    _searchResultDataModel.setParent(this);
    registerQmlTypes();
}

PwDatabaseFacade::~PwDatabaseFacade() {
    clear();
}

void PwDatabaseFacade::registerQmlTypes() {
    qmlRegisterType<PwDatabaseFacade>("org.keepassb", 1, 0, "PwDatabaseFacade");
    qmlRegisterType<PwSearchResultDataModel>("org.keepassb", 1, 0, "PwSearchResultDataModel");
    qmlRegisterType<PwGroup>("org.keepassb", 1, 0, "PwGroup");
    qmlRegisterType<PwGroupV3>("org.keepassb", 1, 0, "PwGroupV3");
    qmlRegisterType<PwGroupV4>("org.keepassb", 1, 0, "PwGroupV4");
    qmlRegisterType<PwEntry>("org.keepassb", 1, 0, "PwEntry");
    qmlRegisterType<PwEntryV3>("org.keepassb", 1, 0, "PwEntryV3");
    qmlRegisterType<PwEntryV4>("org.keepassb", 1, 0, "PwEntryV4");
    qmlRegisterType<PwField>("org.keepassb", 1, 0, "PwField");
    qmlRegisterType<PwAttachment>("org.keepassb", 1, 0, "PwAttachment");
    qmlRegisterType<PwAttachmentDataModel>("org.keepassb", 1, 0, "PwAttachmentDataModel");
}

void PwDatabaseFacade::setLocked(bool locked) {
    if (_locked != locked) {
        _locked = locked;
        emit lockedChanged(locked);
    }
}

void PwDatabaseFacade::lock() {
	clear();
}

void PwDatabaseFacade::clear() {
    _searchResultDataModel.clear();
    releaseDatabase(true);
}

/**
 * Releases memory occupied by current DB instance (if any open),
 * optionally locking it first.
 */
void PwDatabaseFacade::releaseDatabase(bool lockFirst) {
    if (db) {
        if (lockFirst)
            db->lock();
        delete db;
        db = NULL;
    }
}

/**
  * Indicates whether the DB can be edited/saved.
  */
bool PwDatabaseFacade::isEditable() const {
    return !isDemoDatabase();
}

bool PwDatabaseFacade::isDemoDatabase() const {
    return (db) && (db->getDatabaseFilePath() == DEMO_DATABASE_FILE_PATH);
}

/**
 * Returns true when the parameter is the DB root group; false otherwise.
 */
bool PwDatabaseFacade::isRoot(PwGroup* group) const {
    return (group) && (group == getRootGroup());
}

void PwDatabaseFacade::onDbLocked() {
    setLocked(true);
    releaseDatabase(false); // no need to lock, already locked
    emit dbLocked();
}

void PwDatabaseFacade::onDbUnlocked() {
    setLocked(false);
    emit dbUnlocked();
}

void PwDatabaseFacade::unlock(const QString &dbFilePath, const QString &password, const QString &keyFilePath) {
    // delete any leftovers
    clear();

    // Load DB file to memory
    QFile dbFile (dbFilePath);
    if (!dbFile.open(QIODevice::ReadOnly)) {
        LOG("Cannot open DB file: '%s' Error: %d. Message: %s",
                dbFilePath.toUtf8().constData(), dbFile.error(),
                dbFile.errorString().toUtf8().constData());
        emit fileOpenError(tr("Cannot open database file", "An error message shown when the file is not available or cannot be opened."), dbFile.errorString());
        return;
    }
    LOG("DB file open ok");
    QByteArray dbFileData = dbFile.readAll();
    if (dbFile.error() != QFile::NoError) {
        // There was a problem reading the file
        emit fileOpenError(tr("Error loading database file", "An error message shown when the file cannot be loaded/read."), dbFile.errorString());
        dbFile.close();
        return;
    }
    dbFile.close();

    if (dbFileData.isEmpty()) {
        // The file is ok, but empty
        emit dbUnlockError(tr("Database file is empty", "An error message"), PwDatabase::DB_FILE_EMPTY);
        return;
    }

    // Load key file to memory
    QByteArray keyFileData;
    if (!loadKeyFile(keyFilePath, keyFileData))
        return;

    // Get suitable DB processor (KeePass1 vs KeePass2)
    db = createDatabaseInstance(dbFileData);
    if (!db) {
        emit dbUnlockError(tr("Unknown database format", "An error message for unrecognized/unsupported database file structure."), PwDatabase::UNKNOWN_DB_FORMAT);
        return;
    }

    // let DB instance know the original file path
    db->setDatabaseFilePath(dbFilePath);

    // Setup signal forwarding
    connectDatabaseSignals();

    // Do the actual unlocking/loading
    db->load(dbFileData, password, keyFileData);
    Util::safeClear(dbFileData);
    Util::safeClear(keyFileData);
}

/** Establishes signal forwarding from DB instance to this facade */
void PwDatabaseFacade::connectDatabaseSignals() {
    bool res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbLocked()), this, SLOT(onDbLocked())); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbUnlocked()), this, SLOT(onDbUnlocked())); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbLoadError(QString, int)), this, SIGNAL(dbUnlockError(QString, int))); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbSaveError(QString, int)), this, SIGNAL(dbSaveError(QString, int))); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(invalidPasswordOrKey()), this, SIGNAL(invalidPasswordOrKey())); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(progressChanged(int)), this, SIGNAL(progressChanged(int))); Q_ASSERT(res);
    Q_UNUSED(res);
}

/**
 * Loads given key file to the given buffer. No processing, just load or emit error signals.
 * Empty file path is ok.
 * Returns true if successful, otherwise emits fileOpenError signal and returns false.
 */
bool PwDatabaseFacade::loadKeyFile(const QString& keyFilePath, QByteArray& keyFileData) const {
    if (!keyFilePath.isEmpty()) {
        QFile keyFile (keyFilePath);
        if (!keyFile.open(QIODevice::ReadOnly)) {
            LOG("Cannot open key file: '%s' Error: %d. Message: %s",
                    keyFilePath.toUtf8().constData(), keyFile.error(),
                    keyFile.errorString().toUtf8().constData());
            emit fileOpenError(tr("Cannot open key file", "An error message shown when the file is not available or cannot be read. See 'key file' in the supplied thesaurus."), keyFile.errorString());
            return false;
        }
        keyFileData = keyFile.readAll();
        LOG("Key file is: %s", (keyFileData.isEmpty() ? "empty" : "non-empty"));
        keyFile.close();
    } else {
        LOG("Key file not provided");
    }
    return true;
}

PwDatabase* PwDatabaseFacade::createDatabaseInstance(const QByteArray& rawDbData) {
    if (PwDatabaseV3::isSignatureMatch(rawDbData)) {
        return new PwDatabaseV3();
    } else if (PwDatabaseV4::isSignatureMatch(rawDbData)) {
        return new PwDatabaseV4();
    }
    return NULL;
}

Q_INVOKABLE  int PwDatabaseFacade::search(const QString& query) {
    SearchParams params;
    const Settings* settings = Settings::instance();
    params.includeDeleted = settings->isSearchInDeleted();
    params.includeSubgroups = true;
    params.query = query.trimmed();
    params.queryWords = params.query.split(" ", QString::SkipEmptyParts);

    _searchResultDataModel.clear();

    QList<PwEntry*> searchResult;
    int resultSize = db->search(params, searchResult);
    for (int i = 0; i < searchResult.size(); i++) {
         _searchResultDataModel.insert(searchResult.at(i));
    }
    LOG("Found %d entries", resultSize);
    return resultSize;
}

Q_INVOKABLE QString PwDatabaseFacade::getDatabaseFilePath() const {
    if (db) {
        return db->getDatabaseFilePath();
    } else {
        return QString("");
    }
}

/** Returns a timestamped backup file path for the given DB file path. */
QString PwDatabaseFacade::makeBackupFilePath(QString dbFilePath) {
    QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddThhmmss");
    if (dbFilePath.endsWith(".kdbx") || dbFilePath.endsWith(".kdb")) {
        int pos = dbFilePath.lastIndexOf(".");
        return dbFilePath.insert(pos, "_" + timestamp + ".bak");
    } else {
        // unknown file extension (or none) - so just append the timestamp
        return dbFilePath + "_" + timestamp + ".bak";
    }
}

/**
 * Saves changes in the current database.
 * If successful, returns true and emits dbSaved() signal;
 * otherwise returns false and (sometimes) emits an appropriate error signal.
 */
Q_INVOKABLE bool PwDatabaseFacade::save() {
    if (!db) {
        LOG("Cannot save - no DB open");
        return false;
    }
    if (isLocked()) {
        LOG("Cannot save - DB is locked");
        return false;
    }

    emit dbAboutToSave();

    // Encrypt and save to memory
    QByteArray outData;
    if (!db->save(outData)) {
        LOG("DB saving failed");
        return false;
    }

    // Copy original DB to temporary timestamped file
    // (but when we create a DB, the original does not exist, and it is ok)
    QFile dbFile(db->getDatabaseFilePath());
    QString bakFileName = makeBackupFilePath(db->getDatabaseFilePath());
    if (dbFile.exists() && !dbFile.copy(bakFileName)) {
        LOG("Failed to backup the original DB: %s", dbFile.errorString().toUtf8().constData());
        emit fileSaveError(tr("Cannot backup database file. Saving cancelled.", "An error message: failed to make a backup copy of the database file."), dbFile.errorString());
        return false;
    }

    // Write the new data directly to the original file
    if (!dbFile.open(QIODevice::WriteOnly)) {
        LOG("Cannot open DB file: '%s' Error: %d. Message: %s",
                dbFile.fileName().toUtf8().constData(),
                dbFile.error(), dbFile.errorString().toUtf8().constData());
        emit fileSaveError(tr("Cannot save database file", "An error message shown when the database file cannot be saved."), dbFile.errorString());
        return false;
    }
    qint64 bytesWritten = dbFile.write(outData);
    if ((dbFile.error() != QFile::NoError) || (bytesWritten != outData.size())) {
        LOG("Cannot write to DB file. Error: %d. Message: %s", dbFile.error(), dbFile.errorString().toUtf8().constData());
        emit fileSaveError(tr("Cannot write to database file", "An error message shown when the database file cannot be written to."), dbFile.errorString());
        return false;
    }
    if (!dbFile.flush()) {
        LOG("Could not flush the DB file. Error: %d. Message: %s", dbFile.error(), dbFile.errorString().toUtf8().constData());
        emit fileSaveError(tr("Error writing to database file", "An error message shown when the database file cannot be written to."), dbFile.errorString());
        dbFile.close();
        // could not flush -> possibly not completely written -> not saved
        return false;
    }
    dbFile.close();

    if (!Settings::instance()->isBackupDatabaseOnSave()) {
        // Backup switched off, so remove the backup file
        // (there are no backup files for newly created DBs)
        QFile bakFile(bakFileName);
        if (bakFile.exists() && !bakFile.remove()) {
            // Could not delete backup. Not critical and nothing we/user can do about it - so just ignore it silently.
            LOG("Could not remove backup file: %s", bakFile.fileName().toUtf8().constData());
        }
    }
    emit dbSaved();
    return true;
}

/**
 * Changes the master key of the currently opened DB to the specified one and saves the DB.
 * Returns true if successful; otherwise emits a fileOpenError or dbSaveError and returns false.
 */
bool PwDatabaseFacade::changeMasterKey(const QString& password, const QString& keyFilePath) {
    QByteArray keyFileData;
    if (!loadKeyFile(keyFilePath, keyFileData))
        return false;

    if (!db->changeMasterKey(password, keyFileData)) {
        Util::safeClear(keyFileData);
        return false;
    }
    Util::safeClear(keyFileData);

    bool isSuccess = save();
    return isSuccess;
}

/**
 * Creates a sample v4 database with the given path and opens it.
 * THe DB only exists in memory, no file is written.
 * Master key is not initialized, so call changeMasterKey() afterwards.
 * Returns true if successful; false in case of any error.
 */
Q_INVOKABLE bool PwDatabaseFacade::createDatabaseV4(const QString& dbFilePath) {
    // Just in case, make sure there is no DB open
    if (db || !isLocked()) {
        LOG("cannot create new DB, there is one opened");
        return false;
    }

    QString dbName = QFileInfo(dbFilePath).baseName();
    PwDatabaseV4* db4 = PwDatabaseV4::createSampleDatabase(dbName);
    db = db4;

    db4->setDatabaseFilePath(dbFilePath);
    connectDatabaseSignals(); // Setup DB's signal forwarding to this facade
    onDbUnlocked(); // notify QML that the DB is ready

    return true;
}
