#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyProto/rpc_resources.pb.h"
#include "NearbyProtocols/MediumAdvertisement.h"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include "imgui.h"
#include "nanopb-extension/pb_string_extension.h"
#include "pb.h"
#include "pb_decode.h"
#include <chrono>
#include <cstring>

#define RENDER_TABLE_ENTRY(n, f, p) \
    ImGui::TableNextRow();          \
    ImGui::TableSetColumnIndex(0);  \
    ImGui::Text("%s", n);           \
    ImGui::TableNextColumn();       \
    ImGui::Text(f, p);

namespace nearby::client
{

    // NearbyDiscoveredEndpointBase

    NearbyDiscoveredEndpointBase::NearbyDiscoveredEndpointBase() : m_LastLifeSign(), m_Metadata()
    {
        SetReceivedLastLifeSign();
    }

    void NearbyDiscoveredEndpointBase::SetReceivedLastLifeSign()
    {
        m_LastLifeSign = std::chrono::system_clock::now();
    }

    bool NearbyDiscoveredEndpointBase::HasLastLifeSignTimeouted(std::chrono::seconds MaxTimeout)
    {
        return (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()) - m_LastLifeSign.time_since_epoch()) > MaxTimeout;
    }

    // NearbyDiscoveredEndpointBle

    NearbyDiscoveredEndpointBle::NearbyDiscoveredEndpointBle(unsigned char* MacAddress, NearbyDiscoveredAdvertisementBle* Advertisement, nearby_storage_certificate_manager* CertificateManager)
        : NearbyDiscoveredEndpointBase::NearbyDiscoveredEndpointBase(), m_Advertisement(nullptr), m_LastAdvertisementHash(0), m_CertificateManager(CertificateManager), m_ForceReevaluationOfNextAdvertisement(false)
    {
        memcpy(m_MacAddress, MacAddress, sizeof(m_MacAddress));

        SetAdvertisement(Advertisement);
    }

    NearbyDiscoveredEndpointBle::~NearbyDiscoveredEndpointBle()
    {
        if (m_Advertisement)
        {
            delete m_Advertisement;
            m_Advertisement = nullptr;
        }
    }

    void NearbyDiscoveredEndpointBle::RenderDebugFrame()
    {
        if (ImGui::BeginTable("##MediumTable", 2))
        {
            auto medium = m_Advertisement->GetMedium();

            RENDER_TABLE_ENTRY("Version", "%s", nearby_medium_advertisement_ble_version_to_string(medium->version));
            RENDER_TABLE_ENTRY("Is Fast Advertisement", "%s", medium->is_fast_advertisement == true ? "yes" : "no");
            RENDER_TABLE_ENTRY("Is Device Token Present", "%s", medium->is_device_token_present == true ? "yes" : "no");
            RENDER_TABLE_ENTRY("Is Extra Field", "%s", medium->is_device_token_present == true ? "yes" : "no");
            // RENDER_TABLE_ENTRY("Last Life Sign", "%s", fmt::format("{}", this->m_LastLifeSign).data());

            ImGui::EndTable();
        }

        if (ImGui::TreeNode("Share"))
        {
            if (ImGui::BeginTable("##ConnectionTable", 2))
            {
                auto share = m_Advertisement->GetShare();

                RENDER_TABLE_ENTRY("Device Type", "%s", nearby_share_target_type_to_string(share->device_type));
                RENDER_TABLE_ENTRY("Is Visible", "%s", share->is_visible == true ? "yes" : "no");

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }

    void NearbyDiscoveredEndpointBle::SetAdvertisement(NearbyDiscoveredAdvertisementBle* Advertisement, bool FreeOld)
    {
        if (m_Advertisement && FreeOld)
        {
            delete m_Advertisement;
            m_Advertisement = nullptr;
        }

        m_Advertisement = Advertisement;

        bool needsToReevaluateInformation = (m_LastAdvertisementHash != Advertisement->GetHash()) || m_ForceReevaluationOfNextAdvertisement;
        m_LastAdvertisementHash = Advertisement->GetHash();

        if (needsToReevaluateInformation)
        {
            m_Metadata.m_Type = NearbyDiscoveredEndpointType::BLE;
            m_Metadata.m_UniqueId = Advertisement->GetEndpointId();
            m_Metadata.m_Display = fmt::format("{:.4} (Endpoint-Id)", Advertisement->GetConnection()->endpoint_id);

            auto share = GetAdvertisement()->GetShare();

            nearby_storage_decrypted_metadata_buffer* decrypted_metadata_buffer = NULL;

            bool res = nearby_storage_certificate_manager_try_decrypt_encrypted_metadata(
                m_CertificateManager, share->metadata_encryption_key_hash_byte, sizeof(share->metadata_encryption_key_hash_byte), share->salt, sizeof(share->salt), &decrypted_metadata_buffer);

            if (res)
            {
                nearby_sharing_proto_Device parsedDevice = {};

                pb_create_string_decode_callback(&parsedDevice.name);
                pb_create_string_decode_callback(&parsedDevice.display_name);

                pb_istream_t protoInputStream = pb_istream_from_buffer(decrypted_metadata_buffer->data, decrypted_metadata_buffer->length);

                if (pb_decode(&protoInputStream, nearby_sharing_proto_Device_fields, &parsedDevice) == true)
                {
                    std::string displayName = pb_get_string_for_decode_callback(&parsedDevice.display_name);
                    std::string name = pb_get_string_for_decode_callback(&parsedDevice.name);

                    if (displayName.size() || name.size())
                    {
                        m_Metadata.m_Display = fmt::format("{} {}", displayName, (name.empty() ? "" : fmt::format("({})", name)));
                    }
                }

                pb_destroy_string_decode_callback(&parsedDevice.display_name);
                pb_destroy_string_decode_callback(&parsedDevice.name);

                nearby_storage_decrypted_metadata_buffer_destroy(decrypted_metadata_buffer);
            }
            else
            {
                m_ForceReevaluationOfNextAdvertisement = true;
            }
        }
    }

} // namespace nearby::client
