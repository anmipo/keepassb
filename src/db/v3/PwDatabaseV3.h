/*
 * PwDatabaseV3.h
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#ifndef PWDATABASEV3_H_
#define PWDATABASEV3_H_

#include <QObject>
#include "crypto/CryptoManager.h"
#include "db/PwDatabase.h"
#include "db/v3/PwGroupV3.h"
#include "db/v3/PwEntryV3.h"

/**
 * KeePass 1 database header.
 */
class PwHeaderV3: public QObject {
    Q_OBJECT
public:
    const static int HEADER_SIZE = 124;
    const static quint32 SIGNATURE_1 = 0x9AA2D903;
    const static quint32 SIGNATURE_2 = 0xB54BFB65;
    const static quint32 DB_VERSION  = 0x00030003;
    enum Flag {
        FLAG_SHA2     = 1,
        FLAG_RIJNDAEL = 2,
        FLAG_ARCFOUR  = 4,
        FLAG_TWOFISH  = 8
    };
    enum CypherAlgorithm {
        ALGORITHM_AES     = 0,
        ALGORITHM_TWOFISH = 1
    };

private:
    quint32 flags; // used internally by KeePass
    QByteArray masterSeed;
    QByteArray initialVector;
    QByteArray contentHash;
    QByteArray transformSeed;
    quint32 transformRounds;
    quint32 groupCount;
    quint32 entryCount;
    CypherAlgorithm cypherAlgorithm;
public:
    /** Error codes returned by header's methods */
    enum ErrorCode {
        SUCCESS               = 0,
        SIGNATURE_1_MISMATCH  = 1,
        SIGNATURE_2_MISMATCH  = 2,
        UNSUPPORTED_FILE_VERSION = 3,
        UNSUPPORTED_CYPHER       = 4, // only AES/Twofish are supported
        ERROR_RANDOMIZING_IVS    = 5,
    };
    /**
     * Returns a user-friendly error description
     */
    static QString getErrorMessage(ErrorCode errCode);

    PwHeaderV3(QObject* parent=0);
    virtual ~PwHeaderV3();

    /** Reads and parses header data */
    ErrorCode read(QDataStream& data);
    /** Writes header data to the stream */
    ErrorCode write(QDataStream& outStream);

    /** Resets encryption IV, master and transform seeds to (securely) random values. */
    ErrorCode randomizeInitialVectors();
    /** Erases loaded data from memory */
    void clear();

    CypherAlgorithm getCypherAlgorithm() const { return cypherAlgorithm; }
    quint32 getTransformRounds() const { return transformRounds; }
    QByteArray getMasterSeed() const { return masterSeed; }
    QByteArray getTransformSeed() const { return transformSeed; }
    QByteArray getInitialVector() const { return initialVector; }
    QByteArray getContentHash() const { return contentHash; }
    void setContentHash(const QByteArray& contentHash) { this->contentHash = contentHash; }
    quint32 getGroupCount() const { return groupCount; }
    void setGroupCount(quint32 count) { this->groupCount = count; }
    quint32 getEntryCount() const { return entryCount; }
    void setEntryCount(quint32 count) { this->entryCount = count; }
};


class PwDatabaseV3: public PwDatabase {
    Q_OBJECT
public:
    /**
     * Class-specific error codes
     */
    enum ErrorCode {
        SUCCESS = PwDatabase::SUCCESS,
        HEADER_SIZE_ERROR       = 0x10,
        CANNOT_DECRYPT_DB        = 0x30, // generic error
        CANNOT_DECRYPT_DB_AES    = 0x31,
        CANNOT_DECRYPT_DB_TWOFISH= 0x32,
        DECRYPTED_PADDING_ERROR  = 0x33,
        CONTENT_HASHING_ERROR    = 0x34,  // == generic crypto lib error
        DECRYPTED_CHECKSUM_MISMATCH = 0x35,
        NOT_ENOUGH_GROUPS        = 0x40,
        NOT_ENOUGH_ENTRIES       = 0x45,
        ORPHANED_ENTRY_ERROR     = 0x50,
        // write-related error start from 0x80
        CANNOT_ENCRYPT_DB        = 0x80, // generic error
        CANNOT_ENCRYPT_DB_AES    = 0x81,
        CANNOT_ENCRYPT_DB_TWOFISH= 0x82,
    };

private:
    PwHeaderV3 header;
    QByteArray combinedKey;
    QByteArray masterKey;
    QList<PwEntryV3*> metaStreamEntries;
    PwGroupV3* backupGroup;

    /** Reads the encrypted DB; in case of errors emits appropriate signals and returns false. */
    bool readDatabase(const QByteArray& dbBytes);
    /** Decrypts the DB's data using current keys. */
    ErrorCode decryptData(const QByteArray& encryptedData, QByteArray& decryptedData);
    /** Read groups and entries from the decrypted stream and builds their tree */
    ErrorCode readContent(QDataStream& stream);
    /** Reads groups from decrypted stream */
    ErrorCode readAllGroups(QDataStream& stream, QList<PwGroupV3*> &groups);
    /** Reads entries from decrypted stream */
    ErrorCode readAllEntries(QDataStream& stream, QList<PwEntryV3*> &entries);

    /**
     * Puts groups and entries data into the given array (without encryption).
     * Sets groupCount and entryCount to the number of saved groups/entries.
     */
    ErrorCode writeContent(QByteArray& contentData, int& groupCount, int& entryCount);
    /**
     * Encrypts content data using current keys.
     * May change contentData by padding it.
     */
    ErrorCode encryptContent(QByteArray& contentData, QByteArray& encryptedContentData);

protected:
    /** Setter for the combinedKey field */
    virtual void setCombinedKey(const QByteArray& newKey);

    /**
     * Extracts the key from a correctly-formed XML file.
     * Returns true if successful, false otherwise.
     */
    virtual bool processXmlKeyFile(const QByteArray& keyFileData, QByteArray& key) const;

    /** Converts the password string to its raw representation, as of format version's rules */
    virtual QByteArray getPasswordBytes(const QString& password) const;

    /** Combines password and key data into one key */
    virtual bool buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const;

public:
    PwDatabaseV3(QObject* parent=0);
    virtual ~PwDatabaseV3();

    /** Magic timestamp value for never-expiring groups/entries */
    const static QDateTime EXPIRY_DATE_NEVER;

    /** Returns the database format version */
    virtual int getFormatVersion() { return 3; };

    virtual void clear();

    /**
     * Generates a new group ID (guaranteeing it is not being used already)
     */
    qint32 createNewGroupId();
    /**
     * Returns the Backup group of this database.
     * If createIfMissing is true, creates the group if it is missing.
     */
    virtual PwGroup* getBackupGroup(bool createIfMissing = false);

    /**
     * Checks if DB signatures match those of Keepass V3 format.
     */
    static bool isSignatureMatch(const QByteArray& data);

    /**
     * Tries to decrypt the given DB data with given password/key.
     * The progress and the result are communicated asynchronously via signals.
     */
    void load(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData);

    /**
     * Encrypts and writes DB content to the given array.
     * In case of error emits a dbSaveError with error code and returns false.
     */
    bool save(QByteArray& outData);
};

#endif /* PWDATABASEV3_H_ */
