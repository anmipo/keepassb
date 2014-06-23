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

#include "huctx.h"
#include "sbdef.h"
#include "sbreturn.h"

class CryptoManager {
private:
    static CryptoManager* _instance;
    sb_GlobalCtx sbCtx;        // Security Builder Crypto global context

    CryptoManager();
    virtual ~CryptoManager();

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
     * Encrypts data with AES with the specified mode.
     * cipherText must be preallocated to fit the result
     * Returns an SB_* error code.
     */
	int encryptAES(const int mode, const QByteArray& key, const QByteArray& initVector, const QByteArray& plainText, QByteArray& cipherText);

	/**
	 * Decrypts data with AES in CBC mode.
	 * plainText must be preallocated to fit the result
	 * N.B.: does no padding, assumes cypherText size is a multiple of 16.
	 * Returns an SB_* error code.
	 */
	int decryptAES(const QByteArray& key, const QByteArray& initVector, const QByteArray& cypherText, QByteArray& plainText);
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
};

#endif /* CRYPTOMANAGER_H_ */
