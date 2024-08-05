#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyProtocols/MediumAdvertisement.h"
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

    NearbyDiscoveredEndpointBase::NearbyDiscoveredEndpointBase() : m_LastPing()
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

    NearbyDiscoveredEndpointBle::NearbyDiscoveredEndpointBle(NearbyDiscoveredAdvertisementBle* Advertisement)
        : NearbyDiscoveredEndpointBase::NearbyDiscoveredEndpointBase(), m_Advertisement(Advertisement)
    {
        memset(m_MacAddress, 0, sizeof(m_MacAddress));
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

            ImGui::EndTable();
        }
    }

} // namespace nearby::client
