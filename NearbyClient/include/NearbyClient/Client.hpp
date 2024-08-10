#ifndef _NEARBYCLIENT_NEARBYCLIENT_HPP
#define _NEARBYCLIENT_NEARBYCLIENT_HPP

#include "NearbyClient/Bucket.hpp"
#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyClient/Services/OAuth/Authorizer.hpp"
#include "NearbyClient/Services/OAuth/Token.hpp"
#include "NearbyLayers/Bluetooth.h"
#include "NearbyRenderer/Renderer.hpp"
#include <AshLogger/AshLogger.h>
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

        void SaveBucket();

        void CheckForLostEndpointsAndCleanup();

        void OnReceivedOAuthToken(services::OAuthToken Token);
        void OnDiscoveredAdvertisement(unsigned char* MacAddress, bool IsRandomMacAddress, unsigned char* AdvertisementData, unsigned short AdvertisementLength, void* UserParameter);

    private:
        ash::AshLogger m_Logger;
        renderer::NearbyRendererBase* m_Renderer;

        std::filesystem::path m_BucketPath;
        Bucket m_Bucket;

        // OAuth

        services::OAuthorizer m_OAuthorizer;

        // Layers
        nearby_layer_bluetooth_t* m_LayerBluetooth;

        // Layer Endpoints
        std::map<NearbyDiscoveredEndpointId, NearbyDiscoveredEndpointBase*> m_DiscoveredEndpoints;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_NEARBYCLIENT_HPP
