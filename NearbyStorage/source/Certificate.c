#include "NearbyStorage/Certificate.h"
#include <malloc.h>
#include <string.h>

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

    if (instance->metadata_encryption_key_tag_data != NULL)
    {
        free(instance->metadata_encryption_key_tag_data);
    }

    free(instance);
}
