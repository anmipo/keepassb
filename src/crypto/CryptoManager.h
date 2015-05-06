/*
 * CryptoManager.h
 *
 *  Created on: 1 May 2014
 *      Author: Andrei Popleteev
 *
 *  Based on BlackBerry's PasswordSample example distributed under Apache License, Version 2.0.
 *  https://github.com/blackberry/Cascades-Community-Samples/blob/master/PasswordSample/src/SecurityManager.hpp
 */

#ifndef CRYPTOMANAGER_H_
#define CRYPTOMANAGER_H_

#include <QObject>
#include "huctx.h"
#include "sbdef.h"
#include "sbreturn.h"
#include "util/ProgressObserver.h"

class CryptoManager: public QObject {
    Q_OBJECT
private:
    static CryptoManager* _instance;
    sb_GlobalCtx sbCtx;        // Security Builder Crypto global context
    sb_RNGCtx rngCtx;          // RNG context

    int keyTransformLength;
    QByteArray keyTransformInitVectorArray;
    unsigned char* keyTransformIV; // points to keyTransformInitVectorArray.data()
    sb_Params keyTransformAesParams;
    sb_Key keyTransformAesKey;
    sb_Context keyTransformAesContext;
    bool keyTransformInitialized;

    CryptoManager(QObject* parent = 0);
    virtual ~CryptoManager();

    int initRngSeed();

    // releases allocated resources
    void cleanup();

public:
    /**
     * Returns the singleton instance of the class.
     */
	static CryptoManager* instance();

	/**
	 * Initialises security contexts; returns an SB_* error code
	 */
	int init();

	/**
	 * Computes a SHA-256 hash of inputData and puts the result to outputData.
	 * outputData should be preallocated.
	 * Returns an SB_* error code.
	 */
	int sha256(const QByteArray& inputData, QByteArray& outputData);

	/**
	 * Fills 'bytes' with 'size' bytes from a hardware-seeded DRNG.
	 */
	int getRandomBytes(QByteArray& bytes, const int size);

	/**
	 * Encrypts data with AES with the specified mode.
	 * cipherText will be resized to fit the result
	 * Returns an SB_* error code.
	 */
	int encryptAES(const int mode, const QByteArray& key, const QByteArray& initVector,
	        const QByteArray& plainText, QByteArray& cipherText,
	        ProgressObserver* progressObserver = 0);

	/**
	 * Decrypts data with AES in CBC mode.
	 * plainText must be preallocated to fit the result
	 * N.B.: does no padding, assumes cypherText size is a multiple of 16.
	 * Returns an SB_* error code.
	 */
	int decryptAES(const QByteArray& key, const QByteArray& initVector,
	        const QByteArray& cypherText, QByteArray& plainText,
	        ProgressObserver* progressObserver = 0);

	/**
	 * Adds PKCS#7 padding to the array to ensure (mod 16) length.
	 */
	static void addPadding16(QByteArray& data);

	/**
	 * Removes PKCS#7 padding to (mod 16) length. Returns false in case of any error.
	 */
	static bool removePadding16(QByteArray& data);

	// Three methods for key transformation

	/**
	 * Prepares key transformation routine (performKeyTransform).
	 * endKeyTransform() must be called after transformation to free allocated resources.
	 * keySizeBytes specifies the size of keys to transform (SB_AES_128_KEY_BYTES or SB_AES_256_KEY_BYTES)
	 * Returns an SB_* error code.
	 */
	int beginKeyTransform(const QByteArray& key, const int keySizeBytes);
	/**
	 * Performs a key transformation round.
	 * both originalKey and transformedKey must be 16 bytes (SB_AES_128_BLOCK_BYTES) long.
	 * Returns an SB_* error code.
	 */
	int performKeyTransform(const unsigned char* originalKey, unsigned char* transformedKey) const;
	/**
	 * Frees resources allocated by beginKeyTransform().
	 * Returns an SB_* error code.
	 */
	int endKeyTransform();
};

class Salsa20 {
private:
    const static QByteArray SIGMA;
    const static int BLOCK_SIZE = 64;

    bool initialized;
    QByteArray key;
    QByteArray iv;
    quint64 counter;
    QByteArray block;
    int posInBlock;
    void generateBlock();
public:
    Salsa20();
    virtual ~Salsa20();

    /**
     * Initializes the algorithm with the given params, resets internal counters.
     */
    void init(const QByteArray& _key, const QByteArray& _iv);
    /**
     * Fills the buffer with "count" bytes of Salsa20 stream.
     */
    void getBytes(QByteArray& buf, int count);
    /**
     * XORs the buffer with required number of bytes of Salsa20 stream.
     */
    void xorWithNextBytes(QByteArray& buf);
};

#endif /* CRYPTOMANAGER_H_ */
