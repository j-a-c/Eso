#ifndef ESO_CRYPTO_HMAC
#define ESO_CRYPTO_HMAC

#include <string>
#include <openssl/hmac.h>

#include "constants.h"
#include "../global_config/types.h"

/**
 * Implements HMAC-*, where * is one of the allowable modes specified above. If
 * an invalid hash is specified, the default will be SHA-1.
 *
 * @param key   The key to use with the specified hash.
 * @param data  The data to hash.
 * @param hash  The hash to use.
 *
 */
uchar_vec hmac(const std::string key, const uchar_vec data, 
        const int hash) 
{
    // Initialize HMAC context.
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    // The result of the HMAC.
    unsigned char* result;

    // Length will be set in the switch.
    unsigned int len = -1;

    // TODO Implement more hashes.
    switch(hash)
    {
        case (SHA256):
            len = 64;
            HMAC_Init_ex(&ctx, key.c_str(), key.length(), EVP_sha256(), nullptr);
            break;
        case (SHA512):
            len = 128;
            HMAC_Init_ex(&ctx, key.c_str(), key.length(), EVP_sha512(), nullptr);
            break;
        case(SHA1):
        default:
            len = 20;
            HMAC_Init_ex(&ctx, key.c_str(), key.length(), EVP_sha1(), nullptr);
            break;
    }

    result = (unsigned char*)malloc(sizeof(char) * len);

    // Hash data. 
    HMAC_Update(&ctx, &data[0], data.size());
    HMAC_Final(&ctx, result, &len);
    HMAC_CTX_cleanup(&ctx);

    // We are going to return a char_vec.
    uchar_vec res{&result[0], &result[0]+len};
    // Free the allocated memory.
    free(result);

    return res;
}

#endif
