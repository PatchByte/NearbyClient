#include "NearbyStorage/CertificateManager.h"
#include "NearbyStorage/Certificate.h"
#include "NearbyStorage/Crypto.h"
#include "NearbyStorage/HashMap.h"
#include <malloc.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdint.h>
#include <string.h>

#define NEARBY_SHARE_KEY_DERIVED_SIZE 16
#define NEARBY_SHARE_KEY_METADATA_ENCRYPTION_KEY_TAG_SIZE 32

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

// nearby_storage_certificate_manager_try_decrypt_encrypted_metadata

struct nearby_storage_certificate_manager_try_decrypt_encrypted_metadata_user_data
{
    unsigned char* encrypted_metadata_data;
    unsigned long long encrypted_metadata_length;
    unsigned char* salt_data;
    unsigned long long salt_length;
};

bool nearby_storage_certificate_manager_try_decrypt_encrypted_metadata_iter(struct nearby_storage_public_certificate* public_certificate, void* user_data)
{
    struct nearby_storage_certificate_manager_try_decrypt_encrypted_metadata_user_data* udata = user_data;

    unsigned char derived_nearby_share_key_data[NEARBY_SHARE_KEY_DERIVED_SIZE] = {};
    size_t derived_nearby_share_key_length = NEARBY_SHARE_KEY_DERIVED_SIZE;

    unsigned char d1, d2;

    int res = nearby_storage_hkdf_sha256(udata->salt_data, 2, NULL, 0, NULL, 0, derived_nearby_share_key_data, &derived_nearby_share_key_length);

    if (res == 0)
    {
        // Fail here, lets just skip, shouldn't fail, but whatever.
        return true;
    }

    // This is kinda ghetto because I dont know the real size, so I just assume double, max 512 aka single 256
    if (public_certificate->encrypted_metadata_bytes_length > 256)
    {
        return true;
    }

    if (public_certificate->secret_key_length != 32)
    {
        printf("[!] Encountered weird secret_key.\n");
        return true;
    }

    size_t decrypted_metadata_length = public_certificate->encrypted_metadata_bytes_length * 2;
    unsigned char* decrypted_metadata_data = malloc(decrypted_metadata_length);

    memset(decrypted_metadata_data, 0, decrypted_metadata_length);

    decrypted_metadata_length = nearby_storage_aes_ctr_decrypt(udata->encrypted_metadata_data, udata->encrypted_metadata_length,
                                                               public_certificate->secret_key_data, derived_nearby_share_key_data, decrypted_metadata_data);

    {
        unsigned char* hmac_key_data = malloc(NEARBY_SHARE_KEY_METADATA_ENCRYPTION_KEY_TAG_SIZE);
        unsigned char* hmac_md_data = malloc(EVP_MAX_MD_SIZE);
        unsigned int hmac_md_length = EVP_MAX_MD_SIZE;

        memset(hmac_key_data, 0, NEARBY_SHARE_KEY_METADATA_ENCRYPTION_KEY_TAG_SIZE);
        memset(hmac_md_data, 0, EVP_MAX_MD_SIZE);

        if (HMAC(EVP_sha256(), hmac_key_data, NEARBY_SHARE_KEY_METADATA_ENCRYPTION_KEY_TAG_SIZE, decrypted_metadata_data, decrypted_metadata_length, hmac_md_data, NULL) == NULL)
        {
            printf("[!] HMAC fialed\n");
        }

        if (memcmp(hmac_md_data, public_certificate->metadata_encryption_key_tag_data, public_certificate->metadata_encryption_key_tag_length) == 0)
        {
            printf("Found cert\n");
        }

        // HMAC Cleanup

        STOP:

        free(hmac_md_data);
        free(hmac_key_data);
    }

    // End cleanup

    free(decrypted_metadata_data);

    return true;
}

bool nearby_storage_certificate_manager_try_decrypt_encrypted_metadata(struct nearby_storage_certificate_manager* instance, unsigned char* encrypted_metadata_data,
                                                                       unsigned long long encrypted_metadata_length, unsigned char* salt_data, unsigned long long salt_length)
{
    struct nearby_storage_certificate_manager_try_decrypt_encrypted_metadata_user_data udata = {
        .encrypted_metadata_data = encrypted_metadata_data,     //
        .encrypted_metadata_length = encrypted_metadata_length, //
        .salt_data = salt_data,                                 //
        .salt_length = salt_length                              //
    };

    nearby_storage_certificate_manager_iterate_public_certificates(instance, nearby_storage_certificate_manager_try_decrypt_encrypted_metadata_iter, &udata);

    return false;
}
