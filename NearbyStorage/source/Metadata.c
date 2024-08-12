#include "NearbyStorage/Metadata.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

struct nearby_storage_decrypted_metadata_buffer* nearby_storage_decrypted_metadata_buffer_create(unsigned char* data, unsigned long long length)
{
    if (data == NULL || length <= 0)
    {
        return NULL;
    }

    struct nearby_storage_decrypted_metadata_buffer* instance = malloc(sizeof(struct nearby_storage_decrypted_metadata_buffer));

    instance->data = malloc(length);
    instance->length = length;

    memcpy(instance->data, data, length);

    return instance;
}

void nearby_storage_decrypted_metadata_buffer_destroy(struct nearby_storage_decrypted_metadata_buffer* instance)
{
    if (instance->data)
    {
        free(instance->data);
    }

    free(instance);
}
