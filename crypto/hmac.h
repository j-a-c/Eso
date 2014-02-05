#ifndef ESO_CRYPTO_HMAC
#define ESO_CRYPTO_HMAC

#include <string>
#include <openssl/hmac.h>

const int SHA1 = 1;

/**
 * Implements HMAC-*, where * is one of the allowable modes specified above. If
 * an invalid hash is specified, the default will be SHA-1.
 * Because this implementation returns a std::string, it should be
 * reinterpreted as an (unsigned char *) if it is used somewhere. Sample code
 * is shown below.
 *
 * @param key   The key to use with the specified hash.
 * @param data  The data to hash.
 * @param hash  The hash to use.
 *
    std::string key{"012345678910"};
    std::string data{"Hello World!"};

    std::string mac = hmac(key, data, SHA1);
    
    unsigned char *result = (unsigned char *)mac.c_str();

    for (int i = 0; i != mac.length(); i++)
        printf("%02x", result[i]);
    printf("\n");
 */
std::string hmac(const std::string key, const std::string data, 
        const int hash) 
{
    // The result to return.
    std::string res{};

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
        case(SHA1):
        default:
            len = 20;
            HMAC_Init_ex(&ctx, key.c_str(), key.length(), EVP_sha1(), nullptr);
            break;
    }

    result = (unsigned char*)malloc(sizeof(char) * len);

    // Hash data. 
    HMAC_Update(&ctx, (unsigned char*)data.c_str(), data.length());
    HMAC_Final(&ctx, result, &len);
    HMAC_CTX_cleanup(&ctx);

    // Set our return value and free the old one.
    res.append(reinterpret_cast<const char*>(result), len);
    free(result);

    return res;
}

#endif
