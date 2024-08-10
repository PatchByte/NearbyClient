#include "NearbyStorage/CertificateManager.h"
#include "NearbyStorage/Certificate.h"
#include "NearbyStorage/HashMap.h"
#include <malloc.h>
#include <stdint.h>
#include <string.h>

int nearby_storage_public_certificate_compare(const void* a, const void* b, void* udata)
{
    const struct nearby_storage_public_certificate* ca = a;
    const struct nearby_storage_public_certificate* cb = b;

    if (ca->secret_id_length != cb->secret_id_length)
    {
        // Return some sort of error
        return 1;
    }

    return memcmp(ca->secret_id_data, cb->secret_id_data, ca->secret_id_length);
}

uint64_t nearby_storage_public_certificate_hash(const void* item, uint64_t seed0, uint64_t seed1)
{
    return nearby_storage_public_certificate_get_hash((void*)item);
}

void nearby_storage_public_certificate_free(void* item)
{
    nearby_storage_public_certificate_destroy(item);
}

struct nearby_storage_certificate_manager* nearby_storage_certificate_manager_create()
{
    struct nearby_storage_certificate_manager* instance = malloc(sizeof(struct nearby_storage_certificate_manager));

    instance->public_certificates = hashmap_new(sizeof(struct nearby_storage_public_certificate), 0, 0, 0, nearby_storage_public_certificate_hash, nearby_storage_public_certificate_compare,
                                                nearby_storage_public_certificate_free, NULL);

    return instance;
}

void nearby_storage_certificate_manager_destroy(struct nearby_storage_certificate_manager* instance)
{
    hashmap_free(instance->public_certificates);

    free(instance);
}

bool nearby_storage_certificate_manager_add_public_certificate(struct nearby_storage_certificate_manager* instance, struct nearby_storage_public_certificate* public_certificate)
{
    if (hashmap_get(instance->public_certificates, public_certificate) != NULL)
    {
        return false;
    }

    hashmap_set(instance->public_certificates, public_certificate);
    return true;
}

bool nearby_storage_certificate_manager_remove_public_certificate(struct nearby_storage_certificate_manager* instance, unsigned char* secret_id_data, unsigned long long secret_id_length)
{
    const struct nearby_storage_public_certificate* public_certificate =
        hashmap_get(instance->public_certificates, &(struct nearby_storage_public_certificate){.secret_id_data = secret_id_data, .secret_id_length = secret_id_length});

    if (public_certificate == NULL)
    {
        return false;
    }

    hashmap_delete(instance->public_certificates, public_certificate);

    nearby_storage_public_certificate_destroy((void*)public_certificate);

    return true;
}

struct nearby_storage_public_certificate* nearby_storage_certificate_manager_get_public_certificate(struct nearby_storage_certificate_manager* instance, unsigned char* secret_id_data,
                                                                                                    unsigned long long secret_id_length)
{
    return (void*)hashmap_get(instance->public_certificates, &(struct nearby_storage_public_certificate){.secret_id_data = secret_id_data, .secret_id_length = secret_id_length});
}

struct nearby_storage_certificate_manager_iterate_public_certificates_iter_user_data
{
    nearby_storage_certificate_manager_iterate_public_certificate_t iterator_method;
    void* user_data;
};

bool nearby_storage_certificate_manager_iterate_public_certificates_iter_method(const void* item, void* udata)
{
    struct nearby_storage_certificate_manager_iterate_public_certificates_iter_user_data* user_data = udata;
    return user_data->iterator_method((void*)item, user_data->user_data);
}

void nearby_storage_certificate_manager_iterate_public_certificates(struct nearby_storage_certificate_manager* instance,
                                                                    nearby_storage_certificate_manager_iterate_public_certificate_t iterator_method, void* user_data)
{
    struct nearby_storage_certificate_manager_iterate_public_certificates_iter_user_data udata = {};

    udata.iterator_method = iterator_method;
    udata.user_data = user_data;

    hashmap_scan(instance->public_certificates, nearby_storage_certificate_manager_iterate_public_certificates_iter_method, &udata);
}
