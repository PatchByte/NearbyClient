#ifndef _NEARBYSTORAGE_CERTIFICATEMANAGER_H
#define _NEARBYSTORAGE_CERTIFICATEMANAGER_H

#include "NearbyStorage/Certificate.h"
#include "NearbyStorage/HashMap.h"
#ifdef __cplusplus
extern "C"
{
#endif

    struct nearby_storage_certificate_manager
    {
        struct hashmap* public_certificates;
    };

    typedef bool (*nearby_storage_certificate_manager_iterate_public_certificate_t)(struct nearby_storage_public_certificate* public_certificate, void* user_data);

    struct nearby_storage_certificate_manager* nearby_storage_certificate_manager_create();
    void nearby_storage_certificate_manager_destroy(struct nearby_storage_certificate_manager* instance);
    bool nearby_storage_certificate_manager_add_public_certificate(struct nearby_storage_certificate_manager* instance, struct nearby_storage_public_certificate* public_certificate);
    bool nearby_storage_certificate_manager_remove_public_certificate(struct nearby_storage_certificate_manager* instance, unsigned char* secret_id_data, unsigned long long secret_id_length);
    struct nearby_storage_public_certificate* nearby_storage_certificate_manager_get_public_certificate(struct nearby_storage_certificate_manager* instance, unsigned char* secret_id_data,
                                                                                                        unsigned long long secret_id_length);
    void nearby_storage_certificate_manager_iterate_public_certificates(struct nearby_storage_certificate_manager* instance,
                                                                        nearby_storage_certificate_manager_iterate_public_certificate_t iterator_method, void* user_data);

#ifdef __cplusplus
}
#endif

#endif // !_NEARBYSTORAGE_CERTIFICATEMANAGER_H
