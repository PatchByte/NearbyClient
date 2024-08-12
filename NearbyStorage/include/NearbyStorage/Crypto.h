#ifndef _NEARBYSTORAGE_CRYPTO_H
#define _NEARBYSTORAGE_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*!
     * @param[out] derived_key_data is an output.
     * @param[in,out] derived_key_size
     */
    int nearby_storage_hkdf_sha256(unsigned char* secret_data, unsigned long long secret_length, unsigned char* salt_data, unsigned long long salt_length, unsigned char* info_data,
                                   unsigned long long info_length, unsigned char* derived_key_data, size_t* derived_key_size);

    /*!
     * @param[out] decrypted_buffer
     * @return The return is the length. -1 if failed.
     */
    int nearby_storage_aes_ctr_256_decrypt(unsigned char* encrypted_data, unsigned long long encrypted_length, unsigned char* key, unsigned char* iv, unsigned char* decrypted_buffer);

    /*!
     * @param[out] decrypted_data
     */
    int nearby_storage_google_aead_aes_gcm_256_decrypt(unsigned char* encrypted_data, int encrypted_length, unsigned char* aad, int aad_length, unsigned char* tag, unsigned char* key,
                                                       unsigned char* iv, unsigned char* decrypted_data);

#ifdef __cplusplus
}
#endif

#endif // !_NEARBYSTORAGE_CRYPTO_H
