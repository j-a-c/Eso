/*
 * Helper methods for RSA-related stuff.
 */

#ifndef ESO_CENTRAL_CRYPTO_RSA
#define ESO_CENTRAL_CRYPTO_RSA

#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <tuple>

#include "constants.h"
#include "memory.h"
#include "../global_config/types.h"
#include "../logger/logger.h"

/* 
 * Returns the malloc'd buffer, and puts the size of the buffer into the integer
 * pointed to by the second argument. Don't forget to free the returned buffer!
 * From: Secure Programming Cookbook for C and C++ - VM (2003) pg 353.
 */
unsigned char *DER_encode_RSA_public(RSA *rsa, int *len) 
{
    unsigned char *buf, *next;
    *len = i2d_RSAPublicKey(rsa, 0);

    if (!(buf = next = (unsigned char *)malloc(*len))) return 0;
    // If we use buf here, return buf; becomes wrong. 
    i2d_RSAPublicKey(rsa, &next);
    return buf;
}

/* 
 * Returns an RSA structure containing the public key specified by the DER
 * enconding.
 */
RSA *DER_decode_RSA_public(const unsigned char *buf, long len)
{
    return d2i_RSAPublicKey(0, &buf, len);
}

/*
 * Returns the malloc'd buffer (don't forget to free when done!) and puts the
 * size of the buffer into the integer pointed to by the second argument.
 */
unsigned char *DER_encode_RSA_private(RSA *rsa, int *len) 
{
    unsigned char *buf, *next;
    *len = i2d_RSAPrivateKey(rsa, 0);

    if (!(buf = next = (unsigned char *)malloc(*len))) return 0;
    // If we use buf here, return buf; becomes wrong.
    i2d_RSAPrivateKey(rsa, &next);
    return buf;
}

/* 
 * Returns an RSA structure containing the private key specified by the DER
 * enconding.
 */
RSA *DER_decode_RSA_private(const unsigned char *buf, long len)
{
    return d2i_RSAPrivateKey(0, &buf, len);
}

/*
 * The modulus size will be of length bits. Returns a tuple containing the
 * DER-encoded representations of the public and private keys.
 *
 * Tuple is <public_key, private_key>
 */
std::tuple<uchar_vec, uchar_vec> get_new_RSA_pair(int bits)
{
    // TODO most functions in here need error checking..

    // Generate a new RSA key pair
    // http://www.openssl.org/docs/crypto/rsa.html
    RSA *rsa = RSA_new();

    // One of the recommended exponents.
    // http://www.openssl.org/docs/crypto/RSA_generate_key.html
    unsigned long e = 65537;
    // TODO does the BIGNUM need to be freed?
    BIGNUM *exp = BN_new();
    BN_set_word(exp, e);

    // TODO Seed PRNG
    //void RAND_seed(const void *buf, int num);
    //void RAND_add(const void *buf, int num, double entropy);
    int ret = RSA_generate_key_ex(rsa, bits, exp, nullptr);

    // Use DER-encoded representation to represent the public and private keys.
    int public_len;
    unsigned char *public_store = DER_encode_RSA_public(rsa, &public_len);

    int private_len;
    unsigned char *private_store = DER_encode_RSA_private(rsa, &private_len);

    // Frees the RSA structure and its components. 
    // The key is erased before the memory is returned to the system.
    RSA_free(rsa);

    // The encoded public key.
    uchar_vec pub{&public_store[0], &public_store[0]+public_len};
    // The encoded private key.
    uchar_vec pri(&private_store[0], &private_store[0]+private_len);

    // Safely delete the keys.
    free((void*)secure_memset(&public_store[0], 0, public_len));
    free((void*)secure_memset(&private_store[0], 0, private_len));

    return std::make_tuple(pub, pri);
}

/**
 * Encrypts data using the given public key.
 *
 * @param public_key The public RSA key.
 * @param data The data to encrypt.
 */
uchar_vec rsa_encrypt(RSA* public_key, uchar_vec data)
{
    // Will hold the ciphertext.
    unsigned char *cipher = new unsigned char[RSA_size(public_key)];
    memset(cipher,'\0', RSA_size(public_key));

    // TODO seed PRNG
    // Encrypt msg using the public key.
    int encrypted_length = RSA_public_encrypt(data.size(), &data[0],
            cipher, public_key, RSA_PKCS1_OAEP_PADDING);

    if (!encrypted_length)
    {
        Logger::log("Error: encrypted_length was not valid.", LogLevel::Error);
    }

    uchar_vec result{&cipher[0], &cipher[0]+encrypted_length};

    delete [] cipher;

    return result;
}

/**
 *
 */
uchar_vec rsa_decrypt(RSA* private_key, uchar_vec data)
{
    // Will contain the original message.
    unsigned char* orig = new unsigned char[RSA_size(private_key)];

    // Decrypt.
    int orig_size = RSA_private_decrypt(data.size(), &data[0], orig, private_key, RSA_PKCS1_OAEP_PADDING);

    uchar_vec result{&orig[0], &orig[0]+orig_size};

    delete [] orig;

    return result;
}


/**
 * Signs the message using RSA and the specified algorithm.
 *
 * @param private_key The private key to sign with.
 * @param msg The message to sign.
 * @param algo The algorithm to use to sign.
 *
 * @return The signature or an empty char_vec{} if something went wrong.
 */
uchar_vec rsa_sign(RSA *private_key, uchar_vec msg, int algo)
{
    // Allocate the EVP signing key.
    EVP_PKEY *signing_key = EVP_PKEY_new();
    if (!signing_key)
    {
        Logger::log("Error allocating mem for signing_key ", LogLevel::Error);
        return uchar_vec{};
    }

    // Set the signing key.
    if(EVP_PKEY_set1_RSA(signing_key, private_key) <= 0)
    {
        /* Error setting key */ 
        Logger::log("Error setting key.", LogLevel::Error);

        EVP_PKEY_free(signing_key);
    
        return uchar_vec{};
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();

    // Configure the message digest algorithm.
    OpenSSL_add_all_algorithms();

    const EVP_MD *md;

    // Try to obtain the message digest algorithm.
    switch (algo)
    {
        case SHA256:
            md = EVP_get_digestbyname("sha256");
            break;
        default:
            EVP_PKEY_free(signing_key);
            return uchar_vec{};
    }

    // Check to ensure we have obtained the message algorithm.
    if (!md)
    {
        Logger::log("Unable to get message digest algo.", LogLevel::Error);
        EVP_PKEY_free(signing_key);
        EVP_cleanup();
        EVP_MD_CTX_cleanup(ctx);
        EVP_MD_CTX_destroy(ctx);

        return uchar_vec{};
    }

    // Initalize the EVP sign.
    if (!EVP_SignInit(ctx, md))
    {
        Logger::log("EVP_SignInit: failed.", LogLevel::Error);

        EVP_cleanup();
        EVP_PKEY_free(signing_key);
        EVP_MD_CTX_cleanup(ctx);
        EVP_MD_CTX_destroy(ctx);

        return uchar_vec{};
    } 

    // Update the signature with the message.
    if (!EVP_SignUpdate(ctx, &msg[0], msg.size()))
    {
        Logger::log("EVP_SignUpdate: failed.", LogLevel::Error);

        EVP_cleanup();
        EVP_PKEY_free(signing_key);
        EVP_MD_CTX_cleanup(ctx);
        EVP_MD_CTX_destroy(ctx);

        return uchar_vec{};
    }

    // Will hold the signature and length.
    unsigned char *sig;
    size_t siglen;

    sig = (unsigned char *) malloc(EVP_PKEY_size(signing_key));

    // Check that the allocation worked.
    if (!sig)
    {
        Logger::log("malloc failed", LogLevel::Error);

        EVP_cleanup();
        EVP_PKEY_free(signing_key);
        EVP_MD_CTX_cleanup(ctx);
        EVP_MD_CTX_destroy(ctx);

        return uchar_vec{};
    }

    // Retrieve the signature.
    if (!EVP_SignFinal(ctx, sig, &siglen, signing_key))
    {
        Logger::log("EVP_SignFinal: failed.", LogLevel::Error);

        free(sig);
        EVP_cleanup();
        EVP_PKEY_free(signing_key);
        EVP_MD_CTX_cleanup(ctx);
        EVP_MD_CTX_destroy(ctx);

        return uchar_vec{};
    }

    // Clean up EVP context.
    EVP_cleanup();
    EVP_PKEY_free(signing_key);
    EVP_MD_CTX_cleanup(ctx);
    EVP_MD_CTX_destroy(ctx);

    // Free allocated material.
    free(sig);

    return uchar_vec{&sig[0], &sig[0]+siglen};
}

/**
 * Verifies the signature using RSA and the specified algorithm.
 *
 * @param public_key The public key to use.
 * @param sig The signature to verify.
 * @param msg The message to compare to.
 * @param algo The algorithm to use.
 *
 * @return True if the signature is verified, false otherwise or if an error
 * occurred.
 */
bool rsa_verify(RSA *public_key, uchar_vec sig, uchar_vec msg, int algo)
{
    EVP_PKEY *verify_key = EVP_PKEY_new();

    // Allocate the verify key.
    if (!verify_key)
    {
        Logger::log("Error allocating key.", LogLevel::Error);
        return false;
    }

    // Set the public key.
    if(EVP_PKEY_set1_RSA(verify_key, public_key) <= 0)
    {
        /* Error setting key */ 
        Logger::log("Error setting key.", LogLevel::Error);

        EVP_PKEY_free(verify_key);
        return false;
    }

    // Configure the message digest algorithm.
    OpenSSL_add_all_algorithms();

    const EVP_MD *md;

    // Try to obtain the message digest algorithm.
    switch (algo)
    {
        case SHA256:
            md = EVP_get_digestbyname("sha256");
            break;
        default:
            EVP_PKEY_free(verify_key);
            return false;
    }

    // Check to ensure we have obtained the message digest algorithm.
    if (!md)
    {
        Logger::log("Unable to get message digest algo.", LogLevel::Error);

        EVP_PKEY_free(verify_key);
        EVP_cleanup();

        return false;
    }

    // Now we verify the signature.
    EVP_MD_CTX *ctx = EVP_MD_CTX_create();

    // Check to ensure we have allocated a context.
    if (!ctx)
    {
        Logger::log("Error allocating context.", LogLevel::Error);

        EVP_PKEY_free(verify_key);
        EVP_cleanup();

        return false;
    }

    // Initalize the context with the specified algorithm.
    if(!EVP_VerifyInit(ctx, md))
    {
        Logger::log("Error init verify.", LogLevel::Error);

        // Free everything
        EVP_PKEY_free(verify_key);
        EVP_MD_CTX_cleanup(ctx);
        EVP_MD_CTX_destroy(ctx);
        EVP_cleanup();

        return false;
    }

    // Update the context with our data.
    if(!EVP_VerifyUpdate(ctx, &msg[0], msg.size()))
    {
        Logger::log("Error update verify.", LogLevel::Error);

        // Free everything
        EVP_PKEY_free(verify_key);
        EVP_MD_CTX_cleanup(ctx);
        EVP_MD_CTX_destroy(ctx);
        EVP_cleanup();

        return false;
    }

    bool ret = false;
    
    // Verify the signature.
    if(EVP_VerifyFinal(ctx, &sig[0], sig.size(), verify_key))
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    // Clean up
    EVP_cleanup();
    EVP_PKEY_free(verify_key);
    EVP_MD_CTX_cleanup(ctx);
    EVP_MD_CTX_destroy(ctx);

    return ret;
}

#endif
