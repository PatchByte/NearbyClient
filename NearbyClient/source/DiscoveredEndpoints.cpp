#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyProtocols/MediumAdvertisement.h"
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

    NearbyDiscoveredEndpointBase::NearbyDiscoveredEndpointBase() : m_LastPing(), m_UniqueId()
    {
        DoPing();
    }

    void NearbyDiscoveredEndpointBase::DoPing()
    {
        m_LastPing = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
    }

    bool NearbyDiscoveredEndpointBase::IsPingTimeout(std::chrono::seconds MaxTimeout)
    {
        return (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()) - m_LastPing) > MaxTimeout;
    }

    // NearbyDiscoveredEndpointBle

    NearbyDiscoveredEndpointBle::NearbyDiscoveredEndpointBle(unsigned char* MacAddress, NearbyDiscoveredAdvertisementBle* Advertisement)
        : NearbyDiscoveredEndpointBase::NearbyDiscoveredEndpointBase(), m_Advertisement(Advertisement)
    {
        memcpy(m_MacAddress, MacAddress, sizeof(m_MacAddress));
        m_UniqueId = sfMakeUniqueId(m_MacAddress);
    }

    NearbyDiscoveredEndpointBle::~NearbyDiscoveredEndpointBle()
    {
        if (m_Advertisement)
        {
            delete m_Advertisement;
            m_Advertisement = nullptr;
        }
    }

    std::string NearbyDiscoveredEndpointBle::GetDisplayName()
    {
        #if NEARBYSERVICES_ENABLE_GOOGLE_BACKEND_SUPPORT
        #error "Not implemented yet"
        #endif

        //return m_Advertisement->GetConnection()->endpoint_id;
        return fmt::format("{:.{}} (Endpoint-Id)", m_Advertisement->GetConnection()->endpoint_id, 4);
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

            ImGui::EndTable();
        }
    }

    NearbyDiscoveredEndpointUniqueId NearbyDiscoveredEndpointBle::sfMakeUniqueId(unsigned char* MacAddress)
    {
        NearbyDiscoveredEndpointUniqueId uniqueId = {.m_Build{.m_Type = NearbyDiscoveredEndpointType::BLE}};

        memcpy(&uniqueId.m_Build.m_Ble.m_Mac, MacAddress, sizeof(uniqueId.m_Build.m_Ble.m_Mac));

        return std::move(uniqueId);
    }

} // namespace nearby::client
