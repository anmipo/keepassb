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


class ErrorCodesV4 {
public:
    /** Version-specific error codes */
    enum ErrorCode {
        SUCCESS = PwDatabase::SUCCESS,
        PASSWORD_HASHING_ERROR_1 = 0x10,
        PASSWORD_HASHING_ERROR_2 = 0x11,
        PASSWORD_HASHING_ERROR_3 = 0x12,
        KEY_TRANSFORM_INIT_ERROR = 0x20,
        KEY_TRANSFORM_ERROR_1    = 0x21,
        KEY_TRANSFORM_ERROR_2    = 0x22,
        KEY_TRANSFORM_ERROR_3    = 0x23,
        KEY_TRANSFORM_END_ERROR  = 0x24,
        CANNOT_DECRYPT_DB        = 0x30,
        CANNOT_INIT_SALSA20      = 0x31,
        WRONG_BLOCK_ID           = 0x32,
        BLOCK_HASH_NON_ZERO      = 0x33,
        BLOCK_HASH_MISMATCH      = 0x34,
        XML_PARSING_ERROR        = 0x50,
        XML_META_PARSING_ERROR   = 0x51,
        XML_META_BINARIES_PARSING_ERROR          = 0x52,
        XML_META_MEMORY_PROTECTION_PARSING_ERROR = 0x53,
        XML_META_CUSTOM_DATA_PARSING_ERROR       = 0x54,
        XML_META_CUSTOM_ICONS_PARSING_ERROR      = 0x55,
        XML_META_UNKNOWN_TAG_ERROR               = 0x56,

        XML_TIMES_PARSING_ERROR  = 0x70,
        XML_NO_ROOT_GROUP        = 0x71,
        XML_STRING_VALUE_PARSING_ERROR = 0x72,
        GROUP_LOADING_ERROR            = 0x73,
        ICON_ID_IS_NOT_INTEGER         = 0x74,
        INVALID_ATTACHMENT_REFERENCE   = 0x75,
    };
};

class MemoryProtection: public QObject {
    Q_OBJECT
private:
    bool protectTitle;
    bool protectUserName;
    bool protectPassword;
    bool protectUrl;
    bool protectNotes;

public:
    MemoryProtection(QObject* parent=0);
    virtual ~MemoryProtection();

    void clear();
    QString toString() const;

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml);
};

/**
 * Binary data as stored in DB metadata
 */
class PwBinaryV4: public QObject {
    Q_OBJECT
private:
    QString _id;
    bool _isCompressed;
    QByteArray _data;
public:
    PwBinaryV4(QObject* parent=0);
    virtual ~PwBinaryV4();

    void clear();
    QString toString() const;

    bool readFromStream(QXmlStreamReader& xml);

    QString getId() const { return _id; }
    bool isCompressed() const { return _isCompressed; }
    QByteArray getData() const { return _data; }
};

/**
 * Custom icon in a V4 database
 */
class PwCustomIconV4: public QObject {
    Q_OBJECT
private:
    PwUuid uuid;
    QByteArray data;
public:
    PwCustomIconV4(QObject* parent=0);
    virtual ~PwCustomIconV4();

    void clear();
    QString toString() const;

    /** Reads icon fields from an XML stream. Returns true if successful, false otherwise. */
    bool readFromStream(QXmlStreamReader& xml);

    PwUuid getUuid() const;
    QByteArray getData() const;
};

/**
 * Metadata of the V4 database
 */
class PwDatabaseV4Meta: public QObject {
    Q_OBJECT
private:
    QString generator;
    QString databaseName;
    QDateTime databaseNameChangedTime;
    QString databaseDescription;
    QDateTime databaseDescriptionChangedTime;
    QString defaultUserName;
    QDateTime defaultUserNameChangedTime;
    quint32 maintenanceHistoryDays;
    QString colorString; // database color coded as a CSS-format hex string(e.g. #123456), empty string means transparent
    QDateTime masterKeyChangedTime;
    qint64 masterKeyChangeRec;
    qint64 masterKeyChangeForce;
    MemoryProtection memoryProtection;
    bool recycleBinEnabled;
    PwUuid recycleBinGroupUuid;
    QDateTime recycleBinChangedTime;
    PwUuid entryTemplatesGroupUuid;
    QDateTime entryTemplatesGroupChangedTime;
    qint32 historyMaxItems;
    qint64 historyMaxSize;
    PwUuid lastSelectedGroupUuid;
    PwUuid lastTopVisibleGroupUuid;
    QMap<QString, QString> customData; // a set of key=value pairs

    QMap<PwUuid, PwCustomIconV4*> customIcons;
    QMap<QString, PwBinaryV4*> binaries;

    ErrorCodesV4::ErrorCode readCustomData(QXmlStreamReader& xml);
    ErrorCodesV4::ErrorCode readCustomDataItem(QXmlStreamReader& xml);
    ErrorCodesV4::ErrorCode readBinaries(QXmlStreamReader& xml);
    ErrorCodesV4::ErrorCode readCustomIcons(QXmlStreamReader& xml);

    void debugPrint() const;
public:
    PwDatabaseV4Meta(QObject* parent=0);
    virtual ~PwDatabaseV4Meta();

    void clear();

    ErrorCodesV4::ErrorCode readFromStream(QXmlStreamReader& xml);
    const PwUuid& getRecycleBinGroupUuid() const;
    PwBinaryV4* getBinaryByReference(const QString& ref) const;
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
