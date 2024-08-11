#include "NearbyStorage/Crypto.h"
#include "openssl/aes.h"
#include "openssl/kdf.h"
#include <openssl/err.h>
#include <openssl/evp.h>

#define error_crypto(x)                              \
    printf("error while %s: %s\n", __FUNCTION__, x); \
    exit(-1);

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

int nearby_storage_aes_ctr_decrypt(unsigned char* encrypted_data, unsigned long long encrypted_length, unsigned char* key, unsigned char* iv, unsigned char* decrypted_buffer)
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
        return -1;
    }

    if (EVP_DecryptUpdate(ctx, decrypted_buffer, &outlen, encrypted_data, encrypted_length) != 1)
    {
        return -1;
    }

    if (1 != EVP_DecryptFinal_ex(ctx, decrypted_buffer + outlen, &tmplen))
    {
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);

#endif

    return encrypted_length;
}
