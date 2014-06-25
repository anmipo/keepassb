/*
 * PwDatabaseV4.h
 *
 *  Created on: 1 Jun 2014
 *      Author: Andrei Popleteev
 */

#ifndef PWDATABASEV4_H_
#define PWDATABASEV4_H_

#include <QtXml/QXmlStreamReader>
#include "crypto/CryptoManager.h"
#include "db/PwDatabase.h"
#include "db/v4/PwGroupV4.h"
#include "db/v4/PwEntryV4.h"

/**
 * KeePass 2 database header.
 */
class PwHeaderV4: public QObject {
    Q_OBJECT
private:
    bool initialized;
    int size;
    QMap<quint8, QByteArray> data;

    quint64 transformRounds;
    enum Fields {
        HEADER_END                  = 0,
        HEADER_COMMENT              = 1,
        HEADER_CIPHER_ID            = 2,
        HEADER_COMPRESSION_FLAGS    = 3,
        HEADER_MASTER_SEED          = 4,
        HEADER_TRANSFORM_SEED       = 5,
        HEADER_TRANSFORM_ROUNDS     = 6,
        HEADER_ENCRYPTION_IV        = 7,
        HEADER_PROTECTED_STREAM_KEY = 8,
        HEADER_STREAM_START_BYTES   = 9,
        HEADER_INNER_RANDOM_STREAM_ID = 10
    };
public:
    const static quint32 SIGNATURE_1 = 0x9AA2D903;
    const static quint32 SIGNATURE_2 = 0xB54BFB67;

    enum CompressionAlgorithm {
        NONE = 0,
        GZIP = 1
    };
    enum ErrorCode {
        SUCCESS = 0,
        SIGNATURE_1_MISMATCH,
        SIGNATURE_2_MISMATCH,
        UNSUPPORTED_FILE_VERSION,
        NOT_AES,                    // Not AES cypher
        UNKNOWN_COMPRESSION_ALGORITHM,
        MASTER_SEED_SIZE_MISMATCH,
        TRANSFORM_SEED_SIZE_MISMATCH,
        INITIAL_VECTOR_SIZE_MISMATCH,
        PROTECTED_STREAM_SIZE_MISMATCH,
        NOT_SALSA20
    };
    /**
     * Returns a user-friendly error description
     */
    static QString getErrorMessage(ErrorCode errCode);

    PwHeaderV4();
    virtual ~PwHeaderV4();
    /**
     * Reads and parses header data.
     */
    ErrorCode read(QDataStream& stream);

    quint64 getTransformRounds() const;
    QByteArray getTransformSeed() const;
    QByteArray getMasterSeed() const;
    QByteArray getInitialVector() const;
    QByteArray getStreamStartBytes() const;
    QByteArray getProtectedStreamKey() const;
    bool isCompressed() const;

    /**
     * Header size in bytes
     */
    int sizeInBytes() const;
    /**
     * Erases all content from memory
     */
    void clear();

};

struct PwBinaryV4 {
    bool isCompressed;
    QByteArray data;
};

/**
 * Class for handling KeePass 2 databases.
 */
class PwDatabaseV4: public PwDatabase {
    Q_OBJECT
public:
    const static int UUID_BYTES = 16;
    /**
     * Class-specific error codes
     */
    enum ErrorCode {
        SUCCESS = PwDatabase::SUCCESS,
        PASSWORD_HASHING_ERROR_1 = LAST_COMMON + 1,
        PASSWORD_HASHING_ERROR_2,
        PASSWORD_HASHING_ERROR_3,
        KEY_TRANSFORM_ERROR_1,
        KEY_TRANSFORM_ERROR_2,
        KEY_TRANSFORM_ERROR_3,
        CANNOT_INIT_SALSA20,
        CANNOT_DECRYPT_DB,
        WRONG_BLOCK_ID,
        BLOCK_HASH_NON_ZERO,
        BLOCK_HASH_MISMATCH,
        GZIP_DATA_TOO_SHORT,
        GZIP_INIT_FAIL,
        GZIP_INFLATE_ERROR,
        XML_PARSING_ERROR,
        XML_TIMES_PARSING_ERROR,
        XML_NO_ROOT_GROUP,
        GROUP_LOADING_ERROR,
        ICON_ID_IS_NOT_INTEGER,
        XML_STRING_VALUE_PARSING_ERROR
    };

private:
    PwHeaderV4 header;
    QByteArray combinedKey;
    QByteArray aesKey;
    Salsa20 salsa20;
    QMap<QString, PwBinaryV4*> binaries;
    PwUuid recycleBinGroupUuid;

    // Combines password and key data into one key
    ErrorCode buildCombinedKey(const QString& password, const QByteArray& keyFileData, QByteArray& combinedKey);
    // Calculates the AES encryption key based on the combined key (password + key data)
    // and current header seed values.
    ErrorCode transformKey(const PwHeaderV4& header, const QByteArray& combinedKey, QByteArray& aesKey,
            const int progressFrom, const int progressTo);

    // Reads the encrypted DB; in case of errors emits appropriate signals and returns false.
    bool readDatabase(const QByteArray& dbBytes);
    // Decrypts the DB's data using current keys.
    ErrorCode decryptData(const QByteArray& encryptedData, QByteArray& decryptedData);
    // Extracts data blocks from the decrypted data stream, verifying hashes.
    ErrorCode readBlocks(QDataStream& inputStream, QByteArray& gzipData);
    // Unpacks GZip data.
    ErrorCode inflateGZipData(const QByteArray& gzipData, QByteArray& outData);
    // Configures Salsa20 instance for reading protected values
    ErrorCode initSalsa20();
    // Parses well-formed XML data into instance members.
    ErrorCode parseXml(const QString& xmlString);
    // loads metadata of a database
    ErrorCode loadXmlMetaData(QXmlStreamReader& xml);
    // loads a group and its children.
    ErrorCode loadGroupFromXml(QXmlStreamReader& xml, PwGroupV4& group);
    // Loads an entry.
    ErrorCode loadEntryFromXml(QXmlStreamReader& xml, PwEntryV4& entry);
    // Loads timestamps of a group
    ErrorCode readGroupTimes(QXmlStreamReader& xml, PwGroupV4& group);
    // Loads timestamps of an entry
    ErrorCode readEntryTimes(QXmlStreamReader& xml, PwEntryV4& entry);
    // Loads the history tag of an entry and fills entry's history list
    ErrorCode readEntryHistory(QXmlStreamReader& xml, PwEntryV4& hostEntry);
    // Loads a "String" field of an entry.
    ErrorCode readEntryString(QXmlStreamReader& xml, PwEntryV4& entry);
    // Loads the value of a "String" field of an entry; decrypts protected values.
    ErrorCode readEntryStringValue(QXmlStreamReader& xml, QString& value);

    // Prints a tree of the group and all its children (for debug)
    void debugPrint(const PwGroup* group, int indent) const;
public:
    PwDatabaseV4();
    virtual ~PwDatabaseV4();

    /**
     * Checks if DB signatures match those of Keepass V4 format.
     */
    static bool isSignatureMatch(const QByteArray& rawDbData);

    /**
     * Tries to decrypt the given DB data with given password/key.
     * The progress and the result are communicated asynchronously via signals.
     */
    void unlock(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData);

    /**
     * Erases all loaded/decrypted data
     */
    virtual void clear();

};

#endif /* PWDATABASEV4_H_ */
