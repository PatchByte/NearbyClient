#include "NearbyStorage/Crypto.h"
#include "openssl/aes.h"
#include "openssl/kdf.h"
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

#define error_crypto(x)                              \
    printf("error while %s: %s\n", __FUNCTION__, x); \
    exit(-1);

#define handleErrors()                                          \
    {                                                           \
        printf("error while %s: %i\n", __FUNCTION__, __LINE__); \
        exit(-1);                                               \
    }

int nearby_storage_hkdf_sha256(unsigned char* secret_data, unsigned long long secret_length, unsigned char* salt_data, unsigned long long salt_length, unsigned char* info_data,
                               unsigned long long info_length, unsigned char* derived_key_data, size_t* derived_key_size)
{
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);

    if (!pctx)
    {
        error_crypto("Error creating context");
        return 0;
    }

    if (EVP_PKEY_derive_init(pctx) <= 0)
    {
        error_crypto("Error initializing context");
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0)
    {
        error_crypto("Error setting hash algorithm");
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    if (EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt_data, salt_length) <= 0)
    {
        error_crypto("Error setting salt");
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    if (EVP_PKEY_CTX_set1_hkdf_key(pctx, secret_data, secret_length) <= 0)
    {
        error_crypto("Error setting input key material");
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    if (EVP_PKEY_CTX_add1_hkdf_info(pctx, info_data, info_length) <= 0)
    {
        error_crypto("Error setting info");
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    if (EVP_PKEY_derive(pctx, derived_key_data, derived_key_size) <= 0)
    {
        error_crypto("Error deriving the key");
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    EVP_PKEY_CTX_free(pctx);

    return 1;
}

int nearby_storage_aes_ctr_256_decrypt(unsigned char* encrypted_data, unsigned long long encrypted_length, unsigned char* key, unsigned char* iv, unsigned char* decrypted_buffer)
{
#if 0
    // BoringSSL implementation if needed again.

    AES_KEY aes_key;

    if (AES_set_encrypt_key(key, 32 * 8, &aes_key) != 0)
    {
        error_crypto("AES_set_encrypt_key failed.");
        return -1;
    }

    uint8_t ecount_buf[AES_BLOCK_SIZE] = {0};
    unsigned int block_offset = 0;

    AES_ctr128_encrypt(encrypted_data, decrypted_buffer, encrypted_length, &aes_key, iv, ecount_buf, &block_offset);
#else

    int outlen = 0, tmplen = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, iv) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (EVP_DecryptUpdate(ctx, decrypted_buffer, &outlen, encrypted_data, encrypted_length) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (EVP_DecryptFinal_ex(ctx, decrypted_buffer + outlen, &tmplen) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);

#endif

    return encrypted_length;
}

// Stolen from https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
int nearby_storage_aes_gcm_256_decrypt(unsigned char* encrypted_data, int encrypted_length, unsigned char* aad, int aad_length, unsigned char* tag, unsigned char* key, unsigned char* iv,
                                       unsigned char* decrypted_data)
{
    /*
    printf("encrypted_data:\n");
    for (int i = 0; i < encrypted_length; i++)
    {
        printf("%02x ", encrypted_data[i]);
    }
    printf("\n");

    printf("key:\n");
    for (int i = 0; i < 32; i++)
    {
        printf("%02x ", key[i]);
    }
    printf("\n");

    printf("iv:\n");
    for (int i = 0; i < 12; i++)
    {
        printf("%02x ", iv[i]);
    }
    printf("\n");
    */

    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len;
    int ret;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /* Initialise the decryption operation. */
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL))
        handleErrors();

    /* Initialise key and IV */
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (!EVP_DecryptUpdate(ctx, decrypted_data, &len, encrypted_data, encrypted_length))
        handleErrors();
    plaintext_len = len;

    unsigned char dummy_tag[16] = {};

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, dummy_tag))
    {
        ERR_print_errors_fp(stderr);
    }

    /*
     * Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */

    ret = EVP_DecryptFinal_ex(ctx, decrypted_data + len, &len);

    if (ret == 0)
    {
        ERR_print_errors_fp(stderr);
    }

    printf("ret: %i\n", ret);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0)
    {
        /* Success */
        plaintext_len += len;
        return plaintext_len;
    }
    else
    {

        /* Verify failed */
        return plaintext_len;
    }
}
