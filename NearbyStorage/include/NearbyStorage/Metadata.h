#ifndef _NEARBYSTORAGE_METADATA_H
#define _NEARBYSTORAGE_METADATA_H

#ifdef __cplusplus
extern "C"
{
#endif

    // Kinda unnecessary but, whatever

    struct nearby_storage_decrypted_metadata_buffer
    {
        unsigned char* data;
        unsigned long long length;
    };

    struct nearby_storage_decrypted_metadata_buffer* nearby_storage_decrypted_metadata_buffer_create(unsigned char* data, unsigned long long length);
    void nearby_storage_decrypted_metadata_buffer_destroy(struct nearby_storage_decrypted_metadata_buffer* instance);

#ifdef __cplusplus
}
#endif

#endif // !_NEARBYSTORAGE_METADATA_H
