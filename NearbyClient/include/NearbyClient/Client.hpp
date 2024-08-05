#ifndef _NEARBYCLIENT_NEARBYCLIENT_HPP
#define _NEARBYCLIENT_NEARBYCLIENT_HPP

#include "NearbyLayers/Bluetooth.h"
#include "NearbyRenderer/Renderer.hpp"

namespace nearby::client
{

    class NearbyClient
    {
    public:
        NearbyClient();

        void Run();

        void ApplyStyleGui();
        void RenderGui();

        void OnDiscoveredAdvertisement(unsigned char* MacAddress, bool IsRandomMacAddress, unsigned char* AdvertisementData, unsigned short AdvertisementLength, void* UserParameter);

    private:
        renderer::NearbyRendererBase* m_Renderer;
        nearby_layer_bluetooth_t* m_LayerBluetooth;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_NEARBYCLIENT_HPP
