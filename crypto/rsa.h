/*
 * Helper methods for RSA-related stuff.
 */

#ifndef ESO_CENTRAL_CRYPTO_RSA
#define ESO_CENTRAL_CRYPTO_RSA

#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <tuple>

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
 * DER-encoded representations of the public and private keys as well and the
 * key lengths.
 *
 * Tuple is <public, public_len, private, private_len>
 *
 * The key stores must be freed AND safely erased immediately after use!
 */
std::tuple<unsigned char *, int, unsigned char *, int> get_new_RSA_pair(int bits)
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

    return std::make_tuple(public_store, public_len, 
            private_store, private_len);
}

#endif
