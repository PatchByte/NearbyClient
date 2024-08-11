#include "NearbyClient/Services/NearbyShare.hpp"
#include "NearbyProto/certificate_rpc.pb.h"
#include "NearbyProto/rpc_resources.pb.h"
#include "NearbyStorage/Certificate.h"
#include "fmt/format.h"
#include "nanopb-extension/pb_bytes_extension.h"
#include "nanopb-extension/pb_repeated_extension.h"
#include "nanopb-extension/pb_string_extension.h"
#include "pb_encode.h"
#include <httplib.h>
#include <pb.h>
#include <pb_decode.h>
#include <vector>

namespace nearby::client::services
{

    // Too lazy to code
    // Copied from: https://inversepalindrome.com/blog/how-to-create-a-random-string-in-cpp
    static std::string sfRandomString(std::size_t length)
    {
        static constexpr std::string_view smCharacters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<> distribution(0, smCharacters.size() - 1);

        std::string randomStrng;

        for (std::size_t i = 0; i < length; ++i)
        {
            randomStrng += smCharacters[distribution(generator)];
        }

        return randomStrng;
    }

    NearbyShare::NearbyShare(ash::AshLogger& Logger) : m_Logger(Logger), m_Token(), m_Client("nearbysharing-pa.googleapis.com")
    {
    }

    NearbyShare::ListPublicCertificatesResponse NearbyShare::ListPublicCertificates()
    {
        httplib::Headers headers = {
            {"authorization", fmt::format("Bearer {}", m_Token.GetAccessToken())}, // Pass the access token
            {"Content-Type", "application/x-protobuf"},                            // Minimize the footprint
            {"User-Agent", "Mozilla/5.0"}                                          // Some random user agents
        };

        auto res = m_Client.Get(fmt::format("/v1/users/me/devices/{}/publicCertificates", sfRandomString(10)), headers);

        if (res->status != 200)
        {
            m_Logger.Log("Error", "Failed to fetch public certificates. {}", res->body);
            return {.m_Error = false};
        }

        std::string bodyString = res->body;

        nearby_sharing_proto_ListPublicCertificatesResponse protoListPublicCertificatesResponse = {};
        std::vector<nearby_sharing_proto_PublicCertificate*> protoListPublicCertificateResponsePublicCertificates = {};

        pb_istream_t bodyInputStream = pb_istream_from_buffer(reinterpret_cast<pb_byte_t*>(bodyString.data()), bodyString.length());

        // Pre Proto

        pb_create_string_decode_callback(&protoListPublicCertificatesResponse.next_page_token);
        pb_create_submessage_repeated_decode_callback(&protoListPublicCertificatesResponse.public_certificates,
                                                      pb_create_repeated_array_stream(&protoListPublicCertificateResponsePublicCertificates)->SetMsgDesc(nearby_sharing_proto_PublicCertificate_fields),
                                                      [](void* StructureBuffer)
                                                      {
                                                          nearby_sharing_proto_PublicCertificate* publicCertificate = static_cast<nearby_sharing_proto_PublicCertificate*>(StructureBuffer);

                                                          pb_create_bytes_decode_callback(&publicCertificate->secret_id);
                                                          pb_create_bytes_decode_callback(&publicCertificate->secret_key);
                                                          pb_create_bytes_decode_callback(&publicCertificate->public_key);
                                                          pb_create_bytes_decode_callback(&publicCertificate->metadata_encryption_key);
                                                          pb_create_bytes_decode_callback(&publicCertificate->encrypted_metadata_bytes);
                                                          pb_create_bytes_decode_callback(&publicCertificate->metadata_encryption_key_tag);
                                                      });

        // Mid Decode

        if (pb_decode(&bodyInputStream, nearby_sharing_proto_ListPublicCertificatesResponse_fields, &protoListPublicCertificatesResponse) == false)
        {
            m_Logger.Log("Error", "Failed to decode ListPublicCertificatesResponse.");
            return {.m_Error = true};
        }

        // Post Proto

        if (pb_get_string_for_decode_callback(&protoListPublicCertificatesResponse.next_page_token).empty() == false)
        {
            //! @todo implement this!

            m_Logger.Log("Error", "next_page_token is not empty.");
            exit(-1);
        }

        NearbyShare::ListPublicCertificatesResponse parsedListPublicCertificatesResponse = {.m_Error = false};

        pb_iterate_repeated_callback(&protoListPublicCertificatesResponse.public_certificates, [&parsedListPublicCertificatesResponse] (size_t, void* StructureBuffer)
            {
                nearby_sharing_proto_PublicCertificate* protoPublicCertificate = static_cast<nearby_sharing_proto_PublicCertificate*>(StructureBuffer);
                nearby_storage_public_certificate* parsedPublicCertificate = nearby_storage_public_certificate_create();

                auto secretIdBytes = pb_get_bytes_for_decode_callback(&protoPublicCertificate->secret_id);
                auto secretKeyBytes = pb_get_bytes_for_decode_callback(&protoPublicCertificate->secret_key);
                auto publicKeyBytes = pb_get_bytes_for_decode_callback(&protoPublicCertificate->public_key);
                auto metadataEncryptionKeyBytes = pb_get_bytes_for_decode_callback(&protoPublicCertificate->metadata_encryption_key);
                auto encryptedMetadataBytes = pb_get_bytes_for_decode_callback(&protoPublicCertificate->encrypted_metadata_bytes);
                auto metadataEncryptionKeyTag = pb_get_bytes_for_decode_callback(&protoPublicCertificate->metadata_encryption_key_tag);

                parsedPublicCertificate->start_time = protoPublicCertificate->start_time.seconds;
                parsedPublicCertificate->has_start_time = protoPublicCertificate->has_start_time;
                parsedPublicCertificate->end_time = protoPublicCertificate->end_time.seconds;
                parsedPublicCertificate->has_end_time = protoPublicCertificate->has_end_time;
                parsedPublicCertificate->for_selected_contacts = protoPublicCertificate->for_selected_contacts;
                parsedPublicCertificate->for_self_share = protoPublicCertificate->for_self_share;

                nearby_storage_public_certificate_set_secret_id(parsedPublicCertificate, secretIdBytes.data(), secretIdBytes.size());
                nearby_storage_public_certificate_set_secret_key(parsedPublicCertificate, secretKeyBytes.data(), secretKeyBytes.size());
                nearby_storage_public_certificate_set_public_key(parsedPublicCertificate, publicKeyBytes.data(), publicKeyBytes.size());
                nearby_storage_public_certificate_set_metadata_encryption_key(parsedPublicCertificate, metadataEncryptionKeyBytes.data(), metadataEncryptionKeyBytes.size());
                nearby_storage_public_certificate_set_encrypted_metadata_bytes(parsedPublicCertificate, encryptedMetadataBytes.data(), encryptedMetadataBytes.size());
                nearby_storage_public_certificate_set_metadata_encryption_key_tag(parsedPublicCertificate, metadataEncryptionKeyTag.data(), metadataEncryptionKeyTag.size());

                parsedListPublicCertificatesResponse.m_Ceritficates.push_back(parsedPublicCertificate);

                pb_destroy_bytes_decode_callback(&protoPublicCertificate->secret_id);
                pb_destroy_bytes_decode_callback(&protoPublicCertificate->secret_key);
                pb_destroy_bytes_decode_callback(&protoPublicCertificate->public_key);
                pb_destroy_bytes_decode_callback(&protoPublicCertificate->metadata_encryption_key);
                pb_destroy_bytes_decode_callback(&protoPublicCertificate->encrypted_metadata_bytes);
                pb_destroy_bytes_decode_callback(&protoPublicCertificate->metadata_encryption_key_tag);
            });

        pb_destroy_repeated_decode_callback(&protoListPublicCertificatesResponse.public_certificates);
        pb_destroy_string_decode_callback(&protoListPublicCertificatesResponse.next_page_token);

        return std::move(parsedListPublicCertificatesResponse);
    }

} // namespace nearby::client::services
