#ifndef ESO_CRYPTO_AES
#define ESO_CRYPTO_AES

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "../global_config/types.h"

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
    // Input is in bits! We need to convert to bytes, and then to the amount of
    // bytes we will need to allocate.
    // Previously: size / 8
    int len = size / (8*sizeof(char));

    // Convert size from bits to bytes when allocating memory.
    unsigned char *buf = (unsigned char*) malloc(len);
    if(!buf)
        return nullptr;

    // RAND_bytes puts num cryptographically strong pseudo-random bytes into
    // buf.
    // int RAND_bytes(unsigned char *buf, int num);
    if(!RAND_bytes(buf, len))
        return nullptr;

    // buf has now been initialized. 
    return buf;
}

/*
 * This will encrypt *len bytes of data using AES-CBC mode.
 * 
 * @param size The size of the key in bits.
 */
char_vec aes_encrypt(unsigned char *key, char_vec plaintext, int size)
{
    // Used to keep track of the length of the ciphertext.
    int l = plaintext.size();
    int *len = &l;

    EVP_CIPHER_CTX e;

    EVP_CIPHER_CTX_init(&e);
    switch (size)
    {
        case 128:
            EVP_EncryptInit_ex(&e, EVP_aes_128_cbc(), NULL, key, NULL);
            break;
        case 256:
            EVP_EncryptInit_ex(&e, EVP_aes_256_cbc(), NULL, key, NULL);
            break;
    }

    // The max ciphertext length for n bytes of plaintext is 
    // n + AES_BLOCK_SIZE - 1 bytes.
    int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
    unsigned char *ciphertext = (unsigned char *) malloc(c_len);

    // TODO Allows reusing of 'e' for multiple encryption cycles
    // however... we are only using it for one...
    EVP_EncryptInit_ex(&e, NULL, NULL, NULL, NULL);

    // Update ciphertext, c_len is filled with the length of ciphertext 
    // generated, *len is the size of plaintext in bytes.
    EVP_EncryptUpdate(&e, ciphertext, &c_len, (unsigned char *) &plaintext[0], *len);

    // update ciphertext with the final remaining bytes.
    EVP_EncryptFinal_ex(&e, ciphertext+c_len, &f_len);

    *len = c_len + f_len;

    EVP_CIPHER_CTX_cleanup(&e);

    // Construct the return value.
    char_vec ret{&ciphertext[0], &ciphertext[0]+l};

    // Free allocated memory.
    free(ciphertext);

    return ret;
}

/*
 * This will decrypt *len bytes of ciphertext using AES-CBC mode.
 *
 * @param size The size of the key in bits.
 */
char_vec aes_decrypt(unsigned char *key, char_vec ciphertext, int size)
{
    // Used to keep track of the length of the ciphertext.
    int l = ciphertext.size();
    int *len = &l;

    EVP_CIPHER_CTX e;

    EVP_CIPHER_CTX_init(&e);
    switch (size)
    {
        case 128:
            EVP_DecryptInit_ex(&e, EVP_aes_128_cbc(), NULL, key, NULL);
            break;
        case 256:
            EVP_DecryptInit_ex(&e, EVP_aes_256_cbc(), NULL, key, NULL);
            break;
    }

    // Because we have padding ON, we must allocate an extra cipher block size of memory.
    int p_len = *len, f_len = 0;
    unsigned char *plaintext = (unsigned char *) malloc(p_len + AES_BLOCK_SIZE);

    // TODO Check these params.
    EVP_DecryptInit_ex(&e, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate(&e, plaintext, &p_len, (unsigned char *) &ciphertext[0], *len);
    EVP_DecryptFinal_ex(&e, plaintext+p_len, &f_len);

    *len = p_len + f_len;

    EVP_CIPHER_CTX_cleanup(&e);

    char_vec ret{&plaintext[0], &plaintext[0]+*len};

    // Free allocated memory.
    free(plaintext);

    return ret;
}

#endif
