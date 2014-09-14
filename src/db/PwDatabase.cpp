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

// Tag names for XML-formatted key files
const QString XML_KEYFILE = "KeyFile";
const QString XML_META = "Meta";
const QString XML_KEY = "Key";
const QString XML_DATA = "Data";


PwDatabase::PwDatabase(QObject* parent) : QObject(parent), _dbFilePath("") {
	_rootGroup = NULL;
}

PwDatabase::~PwDatabase() {
	qDebug() << "deleting PwDatabase";
    clear();
}

void PwDatabase::lock() {
    this->clear();
    emit dbLocked();
	qDebug("DB locked");
}

void PwDatabase::clear() {
    if (_rootGroup) {
        _rootGroup->clear();
        delete _rootGroup;
        _rootGroup = NULL;
    }
    Util::safeClear(_dbFilePath);
	qDebug("DB cleared");
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
    QXmlStreamReader xml(keyFileData);
    if (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNextStartElement() && (xml.name() == XML_KEYFILE)) {
            if (xml.readNextStartElement() && (xml.name() == XML_META)) {
                xml.skipCurrentElement(); // skip the Meta element
                if (xml.readNextStartElement() && (xml.name() == XML_KEY)) {
                    if (xml.readNextStartElement() && (xml.name() == XML_DATA)) {
                        QString keyText = xml.readElementText();
                        key = QByteArray::fromBase64(keyText.toLatin1());
                        if (!xml.hasError() && (key.length() == KEY_LENGTH)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    xml.clear();


    // It is something else, just hash it
    if (CryptoManager::instance()->sha256(keyFileData, key) != SB_SUCCESS) {
        qDebug() << "could not hash the key file data";
        return false;
    }
    return true;
}

PwGroup* PwDatabase::getRootGroup() {
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

/*****************************************/
PwSearchResultDataModel::PwSearchResultDataModel(QObject* parent) :
        bb::cascades::QListDataModel<PwEntry*>() {
    // QListDataModel does not take parent in constructor, so set it separately
    setParent(parent);
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
    qmlRegisterUncreatableType<PwEntry>("org.keepassb", 1, 0, "PwEntry", "PwEntry is an abstract class and cannot be instantiated");
    qmlRegisterType<PwGroup>("org.keepassb", 1, 0, "PwGroup");
    qmlRegisterType<PwGroupV3>("org.keepassb", 1, 0, "PwGroupV3");
    qmlRegisterType<PwGroupV4>("org.keepassb", 1, 0, "PwGroupV4");
    qmlRegisterType<PwEntryV3>("org.keepassb", 1, 0, "PwEntryV3");
    qmlRegisterType<PwEntryV4>("org.keepassb", 1, 0, "PwEntryV4");
    qmlRegisterType<PwExtraField>("org.keepassb", 1, 0, "PwExtraField");
    qmlRegisterType<PwAttachment>("org.keepassb", 1, 0, "PwAttachment");
}

void PwDatabaseFacade::setLocked(bool locked) {
    if (_locked != locked) {
        _locked = locked;
        emit lockedChanged(locked);
    }
}

void PwDatabaseFacade::lock() {
    if (!isLocked()) {
        clear();
        emit dbLocked();
    }
}

void PwDatabaseFacade::clear() {
    _searchResultDataModel.clear();
    if (db) {
        db->lock();
        delete db;
        db = NULL;
    }
}

void PwDatabaseFacade::onDbLocked() {
    setLocked(true);
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
        qDebug() << "Cannot open DB file: '" << dbFilePath << "' Error: " << dbFile.error() << ". Message: " << dbFile.errorString();
        emit fileOpenError(tr("Cannot open database file", "An error message shown when the file is not available or cannot be opened."), dbFile.errorString());
        return;
    }
    qDebug() << "DB file open ok";
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
    if (!keyFilePath.isEmpty()) {
        QFile keyFile (keyFilePath);
        if (!keyFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open key file: '" << keyFilePath << "' Error: " << keyFile.error() << ". Message: " << keyFile.errorString();
            emit fileOpenError(tr("Cannot open key file", "An error message shown when the file is not available or cannot be read. See 'key file' in the supplied thesaurus."), keyFile.errorString());
            return;
        }
        keyFileData = keyFile.readAll();
        qDebug() << "Key file read: " << (keyFileData.isEmpty() ? "empty" : "non-empty");
        keyFile.close();
    } else {
        qDebug() << "Key file not provided";
    }

    // Get suitable DB processor (KeePass1 vs KeePass2)
    db = createDatabaseInstance(dbFileData);
    if (!db) {
        emit dbUnlockError(tr("Unknown database format", "An error message for unrecognized/unsupported database file structure."), PwDatabase::UNKNOWN_DB_FORMAT);
        return;
    }

    // let DB instance know the original file path
    db->setDatabaseFilePath(dbFilePath);

    // Setup signal forwarding
    bool res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbLocked()), this, SLOT(onDbLocked())); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbUnlocked()), this, SLOT(onDbUnlocked())); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbLoadError(QString, int)), this, SIGNAL(dbUnlockError(QString, int))); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbSaveError(QString, int)), this, SIGNAL(dbSaveError(QString, int))); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(invalidPasswordOrKey()), this, SIGNAL(invalidPasswordOrKey())); Q_ASSERT(res);
    res = QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(unlockProgressChanged(int)), this, SIGNAL(unlockProgressChanged(int))); Q_ASSERT(res);

    // Initiate the actual unlocking/loading
    db->load(dbFileData, password, keyFileData);
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

    QList<PwEntry*> searchResult;

    _searchResultDataModel.clear();
    int resultSize = db->search(params, searchResult);
    _searchResultDataModel.append(searchResult);
    qDebug("Found %d entries", resultSize);
    return resultSize;
}

Q_INVOKABLE QString PwDatabaseFacade::getDatabaseFilePath() const {
    if (db) {
        return db->getDatabaseFilePath();
    } else {
        return QString("");
    }
}

/**
 * Saves changes in the current database.
 */
Q_INVOKABLE void PwDatabaseFacade::save() {
    if (!db) {
        qDebug() << "Cannot save - no DB open";
        return;
    }
    if (isLocked()) {
        qDebug() << "Cannot save - DB is locked";
        return;
    }

    emit dbAboutToSave();

    // Encrypt and save to memory
    QByteArray outData;
    if (!db->save(outData)) {
        qDebug() << "DB saving failed";
        return;
    }


    // Save to file and check all the errors
    QString tmpFilePath = db->getDatabaseFilePath() + "-save.kdb";
    QFile outDbFile(tmpFilePath);
    if (!outDbFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open DB file: '" << tmpFilePath << "' Error: " << outDbFile.error() << ". Message: " << outDbFile.errorString();
        emit fileSaveError(tr("Cannot save database file", "An error message shown when the database file cannot be saved."), outDbFile.errorString());
        return;
    }
    qint64 bytesWritten = outDbFile.write(outData);
    if ((outDbFile.error() != QFile::NoError) || (bytesWritten != outData.size())) {
        qDebug() << "Cannot write to DB file. Error: " << outDbFile.error() << ". Message: " << outDbFile.errorString();
        emit fileSaveError(tr("Cannot write to database file", "An error message shown when the database file cannot be written to."), outDbFile.errorString());
        return;
    }
    if (!outDbFile.flush()) {
        qDebug() << "Could not flush the DB file. Error: " << outDbFile.error() << ". Message: " << outDbFile.errorString();
        emit fileSaveError(tr("Error writing to database file", "An error message shown when the database file cannot be written to."), outDbFile.errorString());
        // no return, pass through to close()
    }
    outDbFile.close();

    // TODO rename the temporary file to the actual one
    emit dbSaved();
}
