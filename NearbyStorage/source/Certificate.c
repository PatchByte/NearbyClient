#include "NearbyStorage/Certificate.h"
#include "NearbyStorage/HashMap.h"
#include <malloc.h>
#include <string.h>

#define NEARBY_STORAGE_PUBLIC_CERTIFICATE_IMPLEMENT_FIELD(n)                                                                                           \
    void nearby_storage_public_certificate_set_##n(struct nearby_storage_public_certificate* instance, unsigned char* data, unsigned long long length) \
    {                                                                                                                                                  \
        if (instance->n##_data)                                                                                                                        \
        {                                                                                                                                              \
            free(instance->n##_data);                                                                                                                  \
            instance->n##_data = NULL;                                                                                                                 \
            instance->n##_length = 0;                                                                                                                  \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (data || length > 0)                                                                                                                        \
        {                                                                                                                                              \
            instance->n##_data = malloc(length);                                                                                                       \
            instance->n##_length = length;                                                                                                             \
            memcpy(instance->n##_data, data, length);                                                                                                  \
        }                                                                                                                                              \
    }

struct nearby_storage_public_certificate* nearby_storage_public_certificate_create()
{
    struct nearby_storage_public_certificate* instance = malloc(sizeof(struct nearby_storage_public_certificate));

    memset(instance, 0, sizeof(*instance));

    return instance;
}

void nearby_storage_public_certificate_destroy(struct nearby_storage_public_certificate* instance)
{
    if (instance->secret_id_data != NULL)
    {
        free(instance->secret_id_data);
    }

    if (instance->secret_key_data != NULL)
    {
        free(instance->secret_key_data);
    }

    if (instance->public_key_data != NULL)
    {
        free(instance->public_key_data);
    }

    if (instance->metadata_encryption_key_data != NULL)
    {
        free(instance->metadata_encryption_key_data);
    }

    if (instance->encrypted_metadata_bytes_data != NULL)
    {
        free(instance->encrypted_metadata_bytes_data);
    }

    if (instance->metadata_encryption_key_tag_data != NULL)
    {
        free(instance->metadata_encryption_key_tag_data);
    }

    free(instance);
}

uint64_t nearby_storage_public_certificate_get_hash(struct nearby_storage_public_certificate* instance)
{
    return hashmap_sip(instance->secret_id_data, instance->secret_id_length, 0, 0);
}

NEARBY_STORAGE_PUBLIC_CERTIFICATE_IMPLEMENT_FIELD(secret_id);
NEARBY_STORAGE_PUBLIC_CERTIFICATE_IMPLEMENT_FIELD(secret_key);
NEARBY_STORAGE_PUBLIC_CERTIFICATE_IMPLEMENT_FIELD(public_key);
NEARBY_STORAGE_PUBLIC_CERTIFICATE_IMPLEMENT_FIELD(metadata_encryption_key);
NEARBY_STORAGE_PUBLIC_CERTIFICATE_IMPLEMENT_FIELD(encrypted_metadata_bytes);
NEARBY_STORAGE_PUBLIC_CERTIFICATE_IMPLEMENT_FIELD(metadata_encryption_key_tag);
