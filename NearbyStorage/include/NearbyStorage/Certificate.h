#ifndef _NEARBYSTORAGE_CERTIFICATE_H
#define _NEARBYSTORAGE_CERTIFICATE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct nearby_storage_public_certificate
    {
        unsigned char* secret_id_data;
        unsigned long long secret_id_length;
        unsigned char* secret_key_data;
        unsigned long long secret_key_length;
        unsigned char* public_key_data;
        unsigned long long public_key_length;
        unsigned long long start_time;
        bool has_start_time;
        unsigned long long end_time;
        bool has_end_time;
        bool for_selected_contacts;
        unsigned char* metadata_encryption_key_data;
        unsigned long long metadata_encryption_key_length;
        unsigned char* encrypted_metadata_bytes_data;
        unsigned long long encrypted_metadata_bytes_length;
        unsigned char* metadata_encryption_key_tag_data;
        unsigned long long metadata_encryption_key_tag_length;
        bool for_self_share;
    };

    struct nearby_storage_public_certificate* nearby_storage_public_certificate_create();
    void nearby_storage_public_certificate_destroy(struct nearby_storage_public_certificate* instance);
    uint64_t nearby_storage_public_certificate_get_hash(struct nearby_storage_public_certificate* instance);
    bool nearby_storage_public_certificate_is_in_time_frame(struct nearby_storage_public_certificate* instance);
    void nearby_storage_public_certificate_set_secret_id(struct nearby_storage_public_certificate* instance, unsigned char* data, unsigned long long length);
    void nearby_storage_public_certificate_set_secret_key(struct nearby_storage_public_certificate* instance, unsigned char* data, unsigned long long length);
    void nearby_storage_public_certificate_set_public_key(struct nearby_storage_public_certificate* instance, unsigned char* data, unsigned long long length);
    void nearby_storage_public_certificate_set_metadata_encryption_key(struct nearby_storage_public_certificate* instance, unsigned char* data, unsigned long long length);
    void nearby_storage_public_certificate_set_encrypted_metadata_bytes(struct nearby_storage_public_certificate* instance, unsigned char* data, unsigned long long length);
    void nearby_storage_public_certificate_set_metadata_encryption_key_tag(struct nearby_storage_public_certificate* instance, unsigned char* data, unsigned long long length);

#ifdef __cplusplus
}
#endif

#endif // !_NEARBYSTORAGE_CERTIFICATE_H
