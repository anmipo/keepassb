/*
 * CryptoManager.cpp
 *
 *  Created on: 1 May 2014
 *      Author: Andrei
 */

#include "CryptoManager.h"
#include <QDebug>
#include "husha2.h"
#include "hugse56.h"
#include "sbreturn.h"
#include "huaes.h"
#include "huseed.h"
#include "hurandom.h"
#include "util/Util.h"

CryptoManager* CryptoManager::_instance;

// convenience macro: if func returns an SB_ error, log it and return its code
#define RETURN_IF_SB_ERROR(func, msg) {int errCode = (func); if (errCode != SB_SUCCESS) { qDebug() << msg << ". ErrCode:" << errCode; return errCode; }}

CryptoManager::CryptoManager() : keyTransformInitVectorArray(SB_AES_128_BLOCK_BYTES, 0) {
	sbCtx = NULL;
	rngCtx = NULL;
    keyTransformInitialized = false;
}

CryptoManager::~CryptoManager() {
    this->cleanup();
	_instance = NULL;
}

CryptoManager* CryptoManager::instance() {
	if (!_instance)
		_instance = new CryptoManager();
	return _instance;
}


/**
 * Initialises security contexts; returns an SB_* error code
 */
int CryptoManager::init() {
	qDebug() << "CryptoManager::init";

	// Create SB Contexts
	RETURN_IF_SB_ERROR(hu_GlobalCtxCreateDefault(&sbCtx), "CryptoManager error creating SB contexts");

	// Register global context with GSE-C 5.6 Provider
	// GSE = Government Security Edition (SB GSE-C)
	// SB GSE-C is also known as BlackBerry OS Cryptographic Kernel
	RETURN_IF_SB_ERROR(hu_RegisterSbg56(sbCtx), "CryptoManager error calling hu_RegisterSbg56");

	RETURN_IF_SB_ERROR(hu_InitSbg56(sbCtx), "CryptoManager error calling hu_InitSbg56");

	// Init RNG seed using hardware-based randomness
	RETURN_IF_SB_ERROR(hu_RegisterSystemSeed(sbCtx), "CryptoManager error calling hu_RegisterSystemSeed");
	RETURN_IF_SB_ERROR(
	        hu_RngDrbgCreate(HU_DRBG_HASH, 256, false, 0, NULL, NULL, &rngCtx, sbCtx),
	        "CryptoManager error creating RNG");
	RETURN_IF_SB_ERROR(initRngSeed(), "CryptoManager error initializing RNG seed");

	return SB_SUCCESS;
}

void CryptoManager::cleanup() {
	qDebug() << "CryptoManager::cleanup";
	if (keyTransformInitialized) {
	    endKeyTransform();
	    keyTransformInitialized = false;
	}
	hu_RngDrbgDestroy(&rngCtx, sbCtx);
	hu_UninitSbg56(sbCtx);
	hu_GlobalCtxDestroy(&sbCtx);
}

int CryptoManager::initRngSeed() {
    size_t seedLen = 1024;
    unsigned char seedBuf[seedLen];
    RETURN_IF_SB_ERROR(hu_SeedGet(&seedLen, seedBuf, sbCtx), "CryptoManager error calling hu_SeedGet");
    // seedLen returned might be less than requested
    RETURN_IF_SB_ERROR(hu_RngReseed(rngCtx, seedLen, seedBuf, sbCtx), "CryptoManager error calling hu_RngReseed");

    return SB_SUCCESS;
}

/**
 * Computes a SHA-256 hash of inputData and puts the result to outputData.
 * outputData will be resized as required => inputData and outputData must be different!
 * Returns an SB_* error code.
 */
int CryptoManager::sha256(const QByteArray& inputData, QByteArray& outputData) {
	//qDebug() << "CryptoManager makeHash:" << inputData.toHex() << " (" << inputData.size() << ")";

	// Initialize parameters
	const unsigned char* hashInput = reinterpret_cast<const unsigned char*>(inputData.constData());
	outputData.fill(0, SB_SHA256_DIGEST_LEN);

	sb_Context sha256Context;
	RETURN_IF_SB_ERROR(
	        hu_SHA256Begin((size_t) SB_SHA256_DIGEST_LEN, NULL, &sha256Context, sbCtx),
	        "CryptoManager error initialising SHA-256 context");

	// Hash Message
	RETURN_IF_SB_ERROR(
	        hu_SHA256Hash(sha256Context, (size_t) inputData.length(), hashInput, sbCtx),
	        "CryptoManager error creating hash");

	// Complete SHA-256 Hashing
	RETURN_IF_SB_ERROR(
	        hu_SHA256End(&sha256Context, reinterpret_cast<unsigned char*>(outputData.data()), sbCtx),
	        "CryptoManager error completing hashing");

	return SB_SUCCESS;
}

/**
 * Fills 'bytes' with 'size' bytes from a hardware-seeded DRNG.
 */
int CryptoManager::getRandomBytes(QByteArray& bytes, const int size) {
    bytes.resize(size);
    RETURN_IF_SB_ERROR(
            hu_RngGetBytes(rngCtx, bytes.size(), reinterpret_cast<unsigned char*>(bytes.data()), sbCtx),
            "Error from hu_RngGetBytes");
    return SB_SUCCESS;
}

/**
 * Encrypts data with AES with the specified mode.
 * cipherText will be resized to fit the result
 * Returns an SB_* error code.
 */
int CryptoManager::encryptAES(const int mode, const QByteArray& key, const QByteArray& initVector, const QByteArray& plainText, QByteArray& cipherText) {
	sb_Params aesParams;
	RETURN_IF_SB_ERROR(
	        hu_AESParamsCreate(mode, SB_AES_128_BLOCK_BITS, NULL, NULL, &aesParams, sbCtx),
	        "AESParamsCreate failed");

	sb_Key aesKey;
	RETURN_IF_SB_ERROR(
	        hu_AESKeySet(aesParams, SB_AES_256_KEY_BITS,
	                reinterpret_cast<const unsigned char*>(key.constData()), &aesKey, sbCtx),
	         "AESKeySet failed");

	sb_Context aesContext;
	RETURN_IF_SB_ERROR(
	        hu_AESBegin(aesParams, aesKey, SB_AES_128_BLOCK_BYTES,
	                reinterpret_cast<const unsigned char*>(initVector.constData()), &aesContext, sbCtx),
	        "AESBegin failed");

    cipherText.fill(0, plainText.size());

	RETURN_IF_SB_ERROR(
	        hu_AESEncryptMsg(aesParams, aesKey,
	                initVector.length(), reinterpret_cast<const unsigned char*>(initVector.constData()),
	                plainText.length(), reinterpret_cast<const unsigned char*>(plainText.constData()),
	                reinterpret_cast<unsigned char*>(cipherText.data()), sbCtx),
	        "AESEncryptMsg failed");

    RETURN_IF_SB_ERROR(
            hu_AESEnd(&aesContext, sbCtx),
            "AESEnd failed");
    RETURN_IF_SB_ERROR(
            hu_AESKeyDestroy(aesParams, &aesKey, sbCtx),
            "AESKeyDestroy failed");
    RETURN_IF_SB_ERROR(
            hu_AESParamsDestroy(&aesParams, sbCtx),
            "AESParamsDestroy failed");
	return SB_SUCCESS;
}

/**
 * Adds PKCS#7 padding to the array to ensure (mod 16) length.
 */
void CryptoManager::addPadding16(QByteArray& data) {
    int padLength = 16 - data.size() % 16;
    if (padLength == 0) padLength = 16;
    QByteArray padding(padLength, padLength);
    data.append(padding);
}

/**
 * Removes PKCS#7 padding to (mod 16) length. Returns false in case of any error.
 */
bool CryptoManager::removePadding16(QByteArray& data) {
    int length = data.length();
    if (length >= 0) {
		// check if padding length is correct
        int padLength = data.at(length - 1);
        if ((padLength < 1) || (padLength > 16)) {
            qDebug() << "Wrong padding length:" << padLength;
            return false;
        }
        // verify if all padding bytes have correct value
        for (int i = length - padLength; i < length; i++) {
            if (data.at(i) != padLength) {
                qDebug() << "Padding bytes corrupted";
                return false;
            }
        }
        data.chop(padLength); // remove the padding
    }
    return true;
}


/**
 * Prepares key transformation routine (performKeyTransform).
 * endKeyTransform() must be called after transformation to free allocated resources.
 * Returns an SB_* error code.
 */
int CryptoManager::beginKeyTransform(const QByteArray& key, const int keySizeBytes) {
    keyTransformLength = keySizeBytes;
    keyTransformInitVectorArray.fill(0, keySizeBytes);
    keyTransformIV = reinterpret_cast<unsigned char*>(keyTransformInitVectorArray.data());

    RETURN_IF_SB_ERROR(
            hu_AESParamsCreate(SB_AES_ECB, SB_AES_128_BLOCK_BITS, NULL, NULL, &keyTransformAesParams, sbCtx),
            "AESParamsCreate failed");

    RETURN_IF_SB_ERROR(
            hu_AESKeySet(keyTransformAesParams, SB_AES_256_KEY_BITS,
                    reinterpret_cast<const unsigned char*>(key.constData()), &keyTransformAesKey, sbCtx),
             "AESKeySet failed");

    RETURN_IF_SB_ERROR(
            hu_AESBegin(keyTransformAesParams, keyTransformAesKey, SB_AES_128_BLOCK_BYTES,
                    keyTransformIV, &keyTransformAesContext, sbCtx),
            "AESBegin failed");

    keyTransformInitialized = true;
    return SB_SUCCESS;
}

/**
 * Performs a key transformation round.
 * Call beginKeyTransform() to prepare necessary resources first.
 * Call endKeyTransform() to free those resources.
 * both originalKey and transformedKey must be the size set in beginKeyTransform().
 * Returns an SB_* error code.
 */
int CryptoManager::performKeyTransform(const unsigned char* originalKey, unsigned char* transformedKey) const {
    RETURN_IF_SB_ERROR(
            hu_AESEncryptMsg(keyTransformAesParams, keyTransformAesKey,
                    keyTransformLength, keyTransformIV,
                    keyTransformLength, originalKey,
                    transformedKey, sbCtx),
            "AESEncryptMsg failed");
    return SB_SUCCESS;
}

/**
 * Frees resources allocated by beginKeyTransform().
 * Returns an SB_* error code.
 */
int CryptoManager::endKeyTransform() {
    if (keyTransformInitialized) {
        RETURN_IF_SB_ERROR(
                hu_AESEnd(&keyTransformAesContext, sbCtx),
                "AESEnd failed");
        RETURN_IF_SB_ERROR(
                hu_AESKeyDestroy(keyTransformAesParams, &keyTransformAesKey, sbCtx),
                "AESKeyDestroy failed");
        RETURN_IF_SB_ERROR(
                hu_AESParamsDestroy(&keyTransformAesParams, sbCtx),
                "AESParamsDestroy failed");
        keyTransformInitialized = false;
    }
    return SB_SUCCESS;
}

/**
 * Decrypts data with AES in CBC mode.
 * plainText must be preallocated to fit the result
 * N.B.: does no padding, assumes cypherText size is a multiple of 16.
 * Returns an SB_* error code.
 */
int CryptoManager::decryptAES(const QByteArray& key, const QByteArray& initVector, const QByteArray& cypherText, QByteArray& plainText) {
	sb_Params aesParams;
	RETURN_IF_SB_ERROR(
	        hu_AESParamsCreate(SB_AES_CBC, SB_AES_128_BLOCK_BITS, NULL, NULL, &aesParams, sbCtx),
	        "AESParamsCreate failed");

	sb_Key aesKey;
	RETURN_IF_SB_ERROR(
	        hu_AESKeySet(aesParams, SB_AES_256_KEY_BITS,
	                reinterpret_cast<const unsigned char*>(key.constData()), &aesKey, sbCtx),
	        "AESKeySet failed");

	sb_Context aesContext;
	RETURN_IF_SB_ERROR(
	        hu_AESBegin(aesParams, aesKey, SB_AES_128_BLOCK_BYTES,
	                reinterpret_cast<const unsigned char*>(initVector.constData()), &aesContext, sbCtx),
	        "AESBegin failed");

	RETURN_IF_SB_ERROR(
	        hu_AESDecryptMsg(aesParams, aesKey,
	                initVector.length(), reinterpret_cast<const unsigned char*>(initVector.constData()),
	                cypherText.length(), reinterpret_cast<const unsigned char*>(cypherText.constData()),
	                reinterpret_cast<unsigned char*>(plainText.data()), sbCtx),
	        "AESDecrypttMsg failed");

    RETURN_IF_SB_ERROR(
            hu_AESEnd(&aesContext, sbCtx),
            "AESEnd failed");
    RETURN_IF_SB_ERROR(
            hu_AESKeyDestroy(aesParams, &aesKey, sbCtx),
            "AESKeyDestroy failed");
    RETURN_IF_SB_ERROR(
            hu_AESParamsDestroy(&aesParams, sbCtx),
            "AESParamsDestroy failed");
	return SB_SUCCESS;
}


/**
 * Salsa 20 reference implementation by D. J. Bernstein (version 20080912)
 * Taken from http://code.metager.de/source/xref/lib/nacl/20110221/crypto_core/salsa20/ref/core.c
 * Public domain.
 */
quint32 rotate(quint32 u,int c) {
  return (u << c) | (u >> (32 - c));
}

quint32 load_littleendian(const unsigned char *x) {
  return (quint32) (x[0]) \
      | (((quint32) (x[1])) << 8) \
      | (((quint32) (x[2])) << 16) \
      | (((quint32) (x[3])) << 24);
}

static void store_littleendian(unsigned char *x, quint32 u) {
  x[0] = u; u >>= 8;
  x[1] = u; u >>= 8;
  x[2] = u; u >>= 8;
  x[3] = u;
}

int coreSalsa20(unsigned char *out, const unsigned char *in, const unsigned char *k, const unsigned char *c) {
	quint32 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
	quint32 j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;
	int i;

	const int ROUNDS = 20;

	j0 = x0 = load_littleendian(c + 0); // 1634760805
	j1 = x1 = load_littleendian(k + 0);
	j2 = x2 = load_littleendian(k + 4);
	j3 = x3 = load_littleendian(k + 8);
	j4 = x4 = load_littleendian(k + 12);
	j5 = x5 = load_littleendian(c + 4); // 857760878
	j6 = x6 = load_littleendian(in + 0);
	j7 = x7 = load_littleendian(in + 4);
	j8 = x8 = load_littleendian(in + 8);
	j9 = x9 = load_littleendian(in + 12);
	j10 = x10 = load_littleendian(c + 8); // 2036477234
	j11 = x11 = load_littleendian(k + 16);
	j12 = x12 = load_littleendian(k + 20);
	j13 = x13 = load_littleendian(k + 24);
	j14 = x14 = load_littleendian(k + 28);
	j15 = x15 = load_littleendian(c + 12); // 1797285236

	for (i = ROUNDS;i > 0;i -= 2) {
		 x4 ^= rotate( x0+x12, 7);
		 x8 ^= rotate( x4+ x0, 9);
		x12 ^= rotate( x8+ x4,13);
		 x0 ^= rotate(x12+ x8,18);
		 x9 ^= rotate( x5+ x1, 7);
		x13 ^= rotate( x9+ x5, 9);
		 x1 ^= rotate(x13+ x9,13);
		 x5 ^= rotate( x1+x13,18);
		x14 ^= rotate(x10+ x6, 7);
		 x2 ^= rotate(x14+x10, 9);
		 x6 ^= rotate( x2+x14,13);
		x10 ^= rotate( x6+ x2,18);
		 x3 ^= rotate(x15+x11, 7);
		 x7 ^= rotate( x3+x15, 9);
		x11 ^= rotate( x7+ x3,13);
		x15 ^= rotate(x11+ x7,18);
		 x1 ^= rotate( x0+ x3, 7);
		 x2 ^= rotate( x1+ x0, 9);
		 x3 ^= rotate( x2+ x1,13);
		 x0 ^= rotate( x3+ x2,18);
		 x6 ^= rotate( x5+ x4, 7);
		 x7 ^= rotate( x6+ x5, 9);
		 x4 ^= rotate( x7+ x6,13);
		 x5 ^= rotate( x4+ x7,18);
		x11 ^= rotate(x10+ x9, 7);
		 x8 ^= rotate(x11+x10, 9);
		 x9 ^= rotate( x8+x11,13);
		x10 ^= rotate( x9+ x8,18);
		x12 ^= rotate(x15+x14, 7);
		x13 ^= rotate(x12+x15, 9);
		x14 ^= rotate(x13+x12,13);
		x15 ^= rotate(x14+x13,18);
	}

	x0 += j0;
	x1 += j1;
	x2 += j2;
	x3 += j3;
	x4 += j4;
	x5 += j5;
	x6 += j6;
	x7 += j7;
	x8 += j8;
	x9 += j9;
	x10 += j10;
	x11 += j11;
	x12 += j12;
	x13 += j13;
	x14 += j14;
	x15 += j15;

	store_littleendian(out + 0,x0);
	store_littleendian(out + 4,x1);
	store_littleendian(out + 8,x2);
	store_littleendian(out + 12,x3);
	store_littleendian(out + 16,x4);
	store_littleendian(out + 20,x5);
	store_littleendian(out + 24,x6);
	store_littleendian(out + 28,x7);
	store_littleendian(out + 32,x8);
	store_littleendian(out + 36,x9);
	store_littleendian(out + 40,x10);
	store_littleendian(out + 44,x11);
	store_littleendian(out + 48,x12);
	store_littleendian(out + 52,x13);
	store_littleendian(out + 56,x14);
	store_littleendian(out + 60,x15);

	return 0;
}

const QByteArray Salsa20::SIGMA = QByteArray("\x65\x78\x70\x61\x6e\x64\x20\x33\x32\x2d\x62\x79\x74\x65\x20\x6b");

Salsa20::Salsa20() : block(BLOCK_SIZE, 0) {
    initialized = false;
    // The following two are initialized here only to suppres the compiler warning.
    // The actual initialization is in init().
    counter = 0L;
    posInBlock = BLOCK_SIZE;
}

Salsa20::~Salsa20() {
    Util::safeClear(key);
    Util::safeClear(iv);
    Util::safeClear(block);
}

void Salsa20::init(const QByteArray& _key, const QByteArray& _iv) {
    Q_ASSERT(_key.length() == 32);
    Q_ASSERT(_iv.length() == 8);

    key = _key;
    iv = _iv;
    counter = 0L;
    posInBlock = BLOCK_SIZE;
    initialized = true;
}

void Salsa20::generateBlock() {
    Q_ASSERT(initialized);
    QByteArray input;
    input.append(iv);
    char* counterBuf = (char*)(&counter);
    input.append(counterBuf, sizeof(counter));

    coreSalsa20(reinterpret_cast<unsigned char*>(block.data()),
            reinterpret_cast<const unsigned char*>(input.constData()),
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(SIGMA.constData()));
}

void Salsa20::getBytes(QByteArray& buf, int count) {
    buf.resize(count);
    //TODO optimize this, process in chunks
    for (int i = 0; i < count; i++) {
        if (posInBlock == BLOCK_SIZE) {
            generateBlock();
            counter++;
            posInBlock = 0;
        }
        buf[i] = block.at(posInBlock);
        posInBlock++;
    }
}

/**
 * XORs the buffer with required number of bytes of Salsa20 stream.
 */
void Salsa20::xorWithNextBytes(QByteArray& buf) {
    int size = buf.length();

    QByteArray salsaBytes;
    getBytes(salsaBytes, size);

    const char* xorBuf = salsaBytes.constData();
    char* valueBuf = buf.data();
    for (int i = 0; i < size; i++) {
        valueBuf[i] ^= xorBuf[i];
    }
}
