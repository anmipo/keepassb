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

    quint32 getTransformRounds() const;
    QByteArray getMasterSeed() const;
    QByteArray getTransformSeed() const;
    QByteArray getInitialVector() const;
    QByteArray getContentHash() const;
    quint32 getGroupCount() const;
    quint32 getEntryCount() const;
};


class PwDatabaseV3: public PwDatabase {
    Q_OBJECT
private:
    PwHeaderV3 header;
    QByteArray combinedKey;

    // Reads the encrypted DB; in case of errors emits appropriate signals and returns false.
    bool readDatabase(const QByteArray& dbBytes);

public:
    /**
     * Class-specific error codes
     */
    enum ErrorCode {
        SUCCESS = PwDatabase::SUCCESS,
        HEADER_SIZE_ERROR       = 0x10
    };

    PwDatabaseV3(QObject* parent=0);
    virtual ~PwDatabaseV3();

    void clear();

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
