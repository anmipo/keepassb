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
    };
    /**
     * Returns a user-friendly error description
     */
    static QString getErrorMessage(ErrorCode errCode);

    PwHeaderV3(QObject* parent=0);
    virtual ~PwHeaderV3();

    /** Reads and parses header data */
    ErrorCode read(QDataStream& data);

    /** Erases loaded data from memory */
    void clear();

    quint32 getTransformRounds() const { return transformRounds; }
    QByteArray getMasterSeed() const { return masterSeed; }
    QByteArray getTransformSeed() const { return transformSeed; }
    QByteArray getInitialVector() const { return initialVector; }
    QByteArray getContentHash() const { return contentHash; }
    quint32 getGroupCount() const { return groupCount; }
    quint32 getEntryCount() const { return entryCount; }
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
    };

private:
    PwHeaderV3 header;
    QByteArray combinedKey;
    QByteArray aesKey;

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
    /** Reads groups from decrypted stream */
    ErrorCode readAllGroups(QDataStream& stream, const quint32 groupCount);
    /** Reads one group */
    ErrorCode readGroup(QDataStream& stream, PwGroupV3& group);

    /** Reads entries from decrypted stream */
    ErrorCode readAllEntries(QDataStream& stream, const quint32 entryCount);
    /** Reads one entry */
    ErrorCode readEntry(QDataStream& stream, PwEntryV3& entry);

    /** Reads a 5-byte V3-specific timestamp from the stream */
    QDateTime readTimestamp(QDataStream& stream);
protected:
    /** Combines password and key data into one key */
    bool buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const;

public:
    PwDatabaseV3(QObject* parent=0);
    virtual ~PwDatabaseV3();

    virtual void clear();

    /**
     * Checks if DB signatures match those of Keepass V3 format.
     */
    static bool isSignatureMatch(const QByteArray& data);

    /**
     * Tries to decrypt the given DB data with given password/key.
     * The progress and the result are communicated asynchronously via signals.
     */
    void unlock(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData);
};

#endif /* PWDATABASEV3_H_ */
