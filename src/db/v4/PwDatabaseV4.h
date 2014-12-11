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
#include "db/v4/DefsV4.h"
#include "db/v4/PwMetaV4.h"
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
    const static quint32 FILE_VERSION = 0x00030001;
    const static quint32 FILE_VERSION_CRITICAL_MASK = 0xFFFF0000;

    enum CompressionAlgorithm {
        NONE = 0,
        GZIP = 1
    };
    enum ErrorCode {
        SUCCESS               = 0,
        SIGNATURE_1_MISMATCH  = 1,
        SIGNATURE_2_MISMATCH  = 2,
        UNSUPPORTED_FILE_VERSION = 3,
        NOT_AES                  = 4, // Not AES cypher
        UNKNOWN_COMPRESSION_ALGORITHM  = 5,
        MASTER_SEED_SIZE_MISMATCH      = 6,
        TRANSFORM_SEED_SIZE_MISMATCH   = 7,
        INITIAL_VECTOR_SIZE_MISMATCH   = 8,
        PROTECTED_STREAM_SIZE_MISMATCH = 9,
        NOT_SALSA20                    = 10
    };

    /**
     * Returns a user-friendly error description
     */
    static QString getErrorMessage(ErrorCode errCode);

    PwHeaderV4(QObject* parent=0);
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



/**
 * Class for handling KeePass 2 databases.
 */
class PwDatabaseV4: public PwDatabase {
    Q_OBJECT
private:
    PwHeaderV4 header;
    PwDatabaseV4Meta meta;
    QByteArray combinedKey;
    QByteArray aesKey;
    Salsa20 salsa20;

    // Calculates the AES encryption key based on the combined key (password + key data)
    // and current header seed values.
    ErrorCodesV4::ErrorCode transformKey(const PwHeaderV4& header, const QByteArray& combinedKey, QByteArray& aesKey,
            const int progressFrom, const int progressTo);

    // Reads the encrypted DB; in case of errors emits appropriate signals and returns false.
    bool readDatabase(const QByteArray& dbBytes);
    // Decrypts the DB's data using current keys.
    ErrorCodesV4::ErrorCode decryptData(const QByteArray& encryptedData, QByteArray& decryptedData);
    // Extracts data blocks from the decrypted data stream, verifying hashes.
    ErrorCodesV4::ErrorCode readBlocks(QDataStream& inputStream, QByteArray& gzipData);
    // Configures Salsa20 instance for reading protected values
    ErrorCodesV4::ErrorCode initSalsa20();
    // Parses well-formed XML data into instance members.
    ErrorCodesV4::ErrorCode parseXml(const QString& xmlString);
    // loads a group and its children.
    ErrorCodesV4::ErrorCode loadGroupFromXml(QXmlStreamReader& xml, PwGroupV4& group);
    // Loads an entry.
    ErrorCodesV4::ErrorCode loadEntryFromXml(QXmlStreamReader& xml, PwEntryV4& entry);
    // Loads timestamps of a group
    ErrorCodesV4::ErrorCode readGroupTimes(QXmlStreamReader& xml, PwGroupV4& group);
    // Loads timestamps of an entry
    ErrorCodesV4::ErrorCode readEntryTimes(QXmlStreamReader& xml, PwEntryV4& entry);
    // Loads the history tag of an entry and fills entry's history list
    ErrorCodesV4::ErrorCode readEntryHistory(QXmlStreamReader& xml, PwEntryV4& hostEntry);
    // Loads a "String" field of an entry.
    ErrorCodesV4::ErrorCode readEntryString(QXmlStreamReader& xml, PwEntryV4& entry);
    // Loads the value of a "String" field of an entry; decrypts protected values.
    ErrorCodesV4::ErrorCode readEntryStringValue(QXmlStreamReader& xml, QString& value);
    // Loads an entry's binary attachment ("Binary" field of an entry).
    ErrorCodesV4::ErrorCode readEntryAttachment(QXmlStreamReader& xml, PwAttachment& attachment);

    // Prints a tree of the group and all its children (for debug)
    void debugPrint(const PwGroup* group, int indent) const;
protected:
    /**
     * Extracts the key from a correctly-formed XML file.
     * Returns true if successful, false otherwise.
     */
    virtual bool processXmlKeyFile(const QByteArray& keyFileData, QByteArray& key) const;

    /** Combines password and key data into one key */
    virtual bool buildCompositeKey(const QByteArray& passwordKey, const QByteArray& keyFileData, QByteArray& combinedKey) const;
public:
    PwDatabaseV4(QObject* parent=0);
    virtual ~PwDatabaseV4();

    /** Returns the database format version */
    virtual int getFormatVersion() { return 4; };

    /**
     * Returns the Backup group of this database.
     * If createIfMissing is true, creates the group if it is missing.
     * (However, if backup is disabled will not create anything and still return NULL).
     */
    virtual PwGroup* getBackupGroup(bool createIfMissing = false);

    /**
     * Checks if DB signatures match those of Keepass V4 format.
     */
    static bool isSignatureMatch(const QByteArray& rawDbData);

    /**
     * Tries to decrypt and load the given DB data with given password/key.
     * The progress and the result are communicated asynchronously via signals.
     */
    void load(const QByteArray& dbFileData, const QString& password, const QByteArray& keyFileData);

    /**
     * Encrypts and writes DB content to the given array.
     */
    bool save(QByteArray& outData);

    /**
     * Erases all loaded/decrypted data
     */
    virtual void clear();
};


#endif /* PWDATABASEV4_H_ */
