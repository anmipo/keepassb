/*
 * PwDatabaseV3.h
 *
 *  Created on: 16 Aug 2014
 *      Author: Andrei Popleteev
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
private:
    quint32 flags; // used internally by KeePass
    QByteArray masterSeed;
    QByteArray initialVector;
    QByteArray contentHash;
    QByteArray transformSeed;
    quint32 transformRounds;
    quint32 groupCount;
    quint32 entryCount;
public:
    const static int HEADER_SIZE = 124;
    const static quint32 SIGNATURE_1 = 0x9AA2D903;
    const static quint32 SIGNATURE_2 = 0xB54BFB65;
    const static quint32 DB_VERSION  = 0x00030003;
    enum Flags {
        FLAG_SHA2     = 1,
        FLAG_RIJNDAEL = 2,
        FLAG_ARCFOUR  = 4,
        FLAG_TWOFISH  = 8
    };
    enum Algorithms {
        ALGORITHM_AES     = 0,
        ALGORITHM_TWOFISH = 1
    };

    /** Error codes returned by header's methods */
    enum ErrorCode {
        SUCCESS               = 0,
        SIGNATURE_1_MISMATCH  = 1,
        SIGNATURE_2_MISMATCH  = 2,
        UNSUPPORTED_FILE_VERSION = 3,
        NOT_AES                  = 4, // only AES/Rijndael is supported
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
        KEY_TRANSFORM_INIT_ERROR = 0x20,
        KEY_TRANSFORM_ERROR_1    = 0x21,
        KEY_TRANSFORM_ERROR_2    = 0x22,
        KEY_TRANSFORM_ERROR_3    = 0x23,
        KEY_TRANSFORM_END_ERROR  = 0x24,
        CANNOT_DECRYPT_DB        = 0x30,
        DECRYPTED_PADDING_ERROR  = 0x31,
        CONTENT_HASHING_ERROR    = 0x32,  // == generic crypto lib error
        DECRYPTED_CHECKSUM_MISMATCH = 0x33,
        NOT_ENOUGH_GROUPS        = 0x40,
        NOT_ENOUGH_ENTRIES       = 0x45,
        ORPHANED_ENTRY_ERROR     = 0x50,
        // write-related error start from 0x80
    };

private:
    PwHeaderV3 header;
    QByteArray combinedKey;
    QByteArray aesKey;
    QList<PwEntryV3*> metaStreamEntries;

    /** Reads the encrypted DB; in case of errors emits appropriate signals and returns false. */
    bool readDatabase(const QByteArray& dbBytes);
    /**
     * Calculates the AES encryption key based on the combined key (password + key data)
     * and current header seed values.
     */
    ErrorCode transformKey(const QByteArray& combinedKey, QByteArray& aesKey,
            const int progressFrom, const int progressTo);
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

protected:
    /** Combines password and key data into one key */
    bool buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const;
    /** Returns all the DB groups and entries */
    void getAllChildren(QList<PwGroupV3*> &groups, QList<PwEntryV3*> &entries);

public:
    PwDatabaseV3(QObject* parent=0);
    virtual ~PwDatabaseV3();

    /** Returns the database format version */
    virtual int getFormatVersion() { return 3; };

    virtual void clear();

    /**
     * Generates a new group ID (guaranteeing it is not being used already)
     */
    qint32 createNewGroupId();

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
