/*
 * PwDatabase.cpp
 *
 *  Created on: 26 May 2014
 *      Author: andrei.popleteev
 */

#include "PwDatabase.h"
#include <QtXml/QXmlStreamReader>
#include "crypto/CryptoManager.h"
#include "db/v4/PwDatabaseV4.h"
#include "db/PwGroup.h"
#include "db/PwEntry.h"

// Tag names for XML-formatted key files
const QString XML_KEYFILE = "KeyFile";
const QString XML_META = "Meta";
const QString XML_KEY = "Key";
const QString XML_DATA = "Data";


PwDatabase::PwDatabase() : QObject() {
	_rootGroup = NULL;
}

PwDatabase::~PwDatabase() {
	qDebug() << "deleting PwDatabase";
    clear();
}

void PwDatabase::lock() {
    this->clear();
	qDebug("DB locked");
}

void PwDatabase::clear() {
    if (_rootGroup) {
        _rootGroup->clear();
        delete _rootGroup;
        _rootGroup = NULL;
    }
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

int PwDatabase::search(const QString& query, QList<PwEntry*> &searchResult) const {
    Q_ASSERT(_rootGroup != NULL);

    searchResult.clear();
    _rootGroup->filterEntries(query, searchResult, true);
    return searchResult.size();
}

/*****************************************/

PwDatabaseFacade::PwDatabaseFacade() : QObject(), _searchResultDataModel() {
    db = NULL;
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
    qmlRegisterUncreatableType<PwGroup>("org.keepassb", 1, 0, "PwGroup", "PwGroup is an abstract class and cannot be instantiated");
    qmlRegisterType<PwGroupV4>("org.keepassb", 1, 0, "PwGroupV4");
    qmlRegisterType<PwEntryV4>("org.keepassb", 1, 0, "PwEntryV4");
    qmlRegisterType<PwExtraField>("org.keepassb", 1, 0, "PwExtraField");
}

void PwDatabaseFacade::lock() {
    clear();
}

void PwDatabaseFacade::clear() {
    _searchResultDataModel.clear();
    if (db) {
        db->lock();
        delete db;
        db = NULL;
    }
}

void PwDatabaseFacade::unlock(const QString &dbFilePath, const QString &password, const QString &keyFilePath) {
    // delete any leftovers
    clear();

    // Load DB file to memory
    QFile dbFile (dbFilePath);
    if (!dbFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open DB file: " << dbFilePath;
        emit fileOpenError(tr("DB file not found"));
        return;
    }
    qDebug() << "DB file read ok";
    QByteArray dbFileData = dbFile.readAll();
    dbFile.close();

    // Load key file to memory
    QByteArray keyFileData;
    if (!keyFilePath.isEmpty()) {
        QFile keyFile (keyFilePath);
        if (!keyFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open key file: " << keyFilePath;
            emit fileOpenError(tr("Cannot open key file"));
            return;
        }
        qDebug() << "Key file found";
        keyFileData = keyFile.readAll();
        keyFile.close();
    } else {
        qDebug() << "Key file not provided";
    }

    // Get suitable DB processor (KeePass1 vs KeePass2)
    db = createDatabaseInstance(dbFileData);
    if (!db) {
        emit dbUnlockError(tr("Unknown DB format"), PwDatabase::UNKNOWN_DB_FORMAT);
        return;
    }

    // Setup signal forwarding
    QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbUnlocked()), this, SIGNAL(dbUnlocked()));
    QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(dbUnlockError(QString, int)), this, SIGNAL(dbUnlockError(QString, int)));
    QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(invalidPasswordOrKey()), this, SIGNAL(invalidPasswordOrKey()));
    QObject::connect(dynamic_cast<QObject*>(db), SIGNAL(unlockProgressChanged(int)), this, SIGNAL(unlockProgressChanged(int)));

    // Initiate the actual unlocking
    db->unlock(dbFileData, password, keyFileData);
    qDebug() << "DB unlock started";
}

PwDatabase* PwDatabaseFacade::createDatabaseInstance(const QByteArray& rawDbData) {
/*
    // not implemented yet
    if (PwDatabase3::isSignatureMatch(rawData)) {
        return new PwDatabase3();
    }
*/
    if (PwDatabaseV4::isSignatureMatch(rawDbData)) {
        return new PwDatabaseV4();
    }
    return NULL;
}

Q_INVOKABLE  int PwDatabaseFacade::search(const QString& query) {
    _searchResultDataModel.clear();
    QList<PwEntry*> searchResult;
    int resultSize = db->search(query.trimmed(), searchResult);
    _searchResultDataModel.append(searchResult);
    qDebug("Found %d entries", resultSize);
    return resultSize;
//    return 0;
}
