#ifndef ESO_CENTRAL_CRYPTO_AES
#define ESO_CENTRAL_CRYPTO_AES

#include <openssl/rand.h>

/*
 * Uses the method from NIST.SP.800-133 where the key = U ^ V. In this case, 
 * U is buf and V = 0; A different V can be supplied later as long as it is
 * independent of U.
 *
 * Returns a buffer malloc'd and filled with the key. Safely delete memory 
 * before returning it to the OS! Size is the size of the key in bits.
 *
 * The returns nullptr if not successful.
 */
unsigned char *get_new_AES_key(int size)
{
    // Convert size from bits to bytes when allocating memory.
    unsigned char *buf = (unsigned char*) malloc(size / 8);
    if(!buf)
        return nullptr;

    // RAND_bytes puts num cryptographically strong pseudo-random bytes into
    // buf.
    // int RAND_bytes(unsigned char *buf, int num);
    if(!RAND_bytes(buf, size / 8))
        return nullptr;

    // buf has now been initialized. 
    return buf;
}

#endif
