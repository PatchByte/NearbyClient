#ifndef _NEARBYCLIENT_NEARBYCLIENT_HPP
#define _NEARBYCLIENT_NEARBYCLIENT_HPP

#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyLayers/Bluetooth.h"
#include "NearbyRenderer/Renderer.hpp"
#include <cstdint>
#include <map>

namespace nearby::client
{

    class NearbyClient
    {
    public:
        NearbyClient();

        void Run();

        void ApplyStyleGui();
        void RenderGui();

        void CheckForLostEndpointsAndCleanup();

        void OnDiscoveredAdvertisement(unsigned char* MacAddress, bool IsRandomMacAddress, unsigned char* AdvertisementData, unsigned short AdvertisementLength, void* UserParameter);

    private:
        renderer::NearbyRendererBase* m_Renderer;

        // Layers
        nearby_layer_bluetooth_t* m_LayerBluetooth;

        // Layer Endpoints
        std::map<uint64_t, NearbyDiscoveredEndpointBase*> m_DiscoveredEndpoints;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_NEARBYCLIENT_HPP
