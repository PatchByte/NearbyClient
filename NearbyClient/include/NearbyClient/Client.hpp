#ifndef _NEARBYCLIENT_NEARBYCLIENT_HPP
#define _NEARBYCLIENT_NEARBYCLIENT_HPP

#include "NearbyClient/Bucket.hpp"
#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyClient/Services/NearbyShare.hpp"
#include "NearbyClient/Services/OAuth/Authorizer.hpp"
#include "NearbyClient/Services/OAuth/Token.hpp"
#include "NearbyLayers/Bluetooth.h"
#include "NearbyRenderer/Renderer.hpp"
#include "NearbyStorage/CertificateManager.h"
#include <AshLogger/AshLogger.h>
#include <map>
#include <mutex>

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

        void CheckIfNeedToRefreshToken();
        void FetchPublicCertificates();
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

        // Services
        services::NearbyShare m_NearbyShare;
        nearby_storage_certificate_manager* m_CertificateManager;

        // Layers
        nearby_layer_bluetooth_t* m_LayerBluetooth;

        // Layer Endpoints
        std::mutex m_DiscoveredEndpointsMutex;
        std::map<NearbyDiscoveredEndpointId, NearbyDiscoveredEndpointBase*> m_DiscoveredEndpoints;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_NEARBYCLIENT_HPP
