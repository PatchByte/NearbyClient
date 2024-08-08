#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyClient/DiscoveredEndpointId.hpp"
#include "NearbyProtocols/MediumAdvertisement.h"
#include "fmt/chrono.h"
#include "fmt/compile.h"
#include "fmt/format.h"
#include "imgui.h"
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

    NearbyDiscoveredEndpointBle::NearbyDiscoveredEndpointBle(unsigned char* MacAddress, NearbyDiscoveredAdvertisementBle* Advertisement)
        : NearbyDiscoveredEndpointBase::NearbyDiscoveredEndpointBase(), m_Advertisement(nullptr)
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
            RENDER_TABLE_ENTRY("Last Life Sign", "%s", fmt::format("{}", this->m_LastLifeSign).data());

            ImGui::EndTable();
        }

        if (ImGui::TreeNode("Share"))
        {
            if (ImGui::BeginTable("##ConnectionTable", 2))
            {
                auto share = m_Advertisement->GetShare();

                RENDER_TABLE_ENTRY("Device Type", "%s", nearby_share_target_type_to_string(share->device_type));

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

        m_Metadata.m_Type = NearbyDiscoveredEndpointType::BLE;
        m_Metadata.m_UniqueId = Advertisement->GetEndpointId();
        m_Metadata.m_Display = fmt::format("{:.4} (Endpoint-Id)", Advertisement->GetConnection()->endpoint_id);
    }

} // namespace nearby::client
