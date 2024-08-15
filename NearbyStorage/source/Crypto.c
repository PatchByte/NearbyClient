#include "NearbyStorage/Crypto.h"
#include "openssl/aes.h"
#include "openssl/kdf.h"
#include <mbedtls/aes.h>
#include <mbedtls/cipher.h>
#include <mbedtls/gcm.h>
#include <mbedtls/hkdf.h>
#include <mbedtls/md.h>

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
    const mbedtls_md_info_t* md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    return mbedtls_hkdf(md, salt_data, salt_length, secret_data, secret_length, info_data, info_length, derived_key_data, *derived_key_size);
}

int nearby_storage_aes_ctr_256_decrypt(unsigned char* encrypted_data, unsigned long long encrypted_length, unsigned char* key, unsigned char* iv, unsigned char* decrypted_buffer)
{
    // BoringSSL
#if 0

    AES_KEY aes_key;

    if (AES_set_encrypt_key(key, 32 * 8, &aes_key) != 0)
    {
        error_crypto("AES_set_encrypt_key failed.");
        return -1;
    }

    uint8_t ecount_buf[AES_BLOCK_SIZE] = {0};
    unsigned int block_offset = 0;

    AES_ctr128_encrypt(encrypted_data, decrypted_buffer, encrypted_length, &aes_key, iv, ecount_buf, &block_offset);
#endif
    // OpenSSL
#if 0

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
    // MbedTLS

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    if (mbedtls_aes_setkey_dec(&aes, key, 256) <= 0)
    {
        return -1;
    }

    size_t nonce_counter_offset = 0;
    unsigned char stream_block[16] = {};

    if (mbedtls_aes_crypt_ctr(&aes, encrypted_length, &nonce_counter_offset, iv, stream_block, encrypted_data, decrypted_buffer) <= 0)
    {
        return -2;
    }

    mbedtls_aes_free(&aes);

    return encrypted_length;
}

// Stolen from https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
int nearby_storage_google_aead_aes_gcm_256_decrypt(unsigned char* encrypted_data, int encrypted_length, unsigned char* aad, int aad_length, unsigned char* tag, unsigned char* key, unsigned char* iv,
                                                   unsigned char* decrypted_data)
{
#if 0
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

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
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
        return -1;
    }
#endif

    mbedtls_gcm_context gcm;

    mbedtls_gcm_init(&gcm);

    if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256) != 0)
    {
        return -1;
    }

    if (mbedtls_gcm_auth_decrypt(&gcm, encrypted_length, iv, 12, aad, aad_length, tag, 16, encrypted_data, decrypted_data) != 0)
    {
        return -2;
    }

    mbedtls_gcm_free(&gcm);

    return encrypted_length;
}

int nearby_storage_hmac_sha_256(unsigned char* key_data, int key_length, unsigned char* input_data, size_t input_length, unsigned char* output_data)
{
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_md_hmac_starts(&ctx, key_data, key_length);
    mbedtls_md_hmac_update(&ctx, input_data, input_length);

    return mbedtls_md_hmac_finish(&ctx, output_data) == 0;
}
