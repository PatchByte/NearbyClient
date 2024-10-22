#ifndef _NEARBYCLIENT_DISCOVERED_HPP
#define _NEARBYCLIENT_DISCOVERED_HPP

#include "Ash/AshBuffer.h"
#include "Ash/AshCRC32.h"
#include "NearbyClient/DiscoveredEndpointId.hpp"
#include "NearbyProtocols/ConnectionAdvertisement.h"
#include "NearbyProtocols/MediumAdvertisement.h"
#include "NearbyProtocols/ShareAdvertisement.h"
#include <stdlib.h>

namespace nearby::client
{
    class NearbyDiscoveredAdvertisementBle
    {
    public:
        NearbyDiscoveredAdvertisementBle();
        ~NearbyDiscoveredAdvertisementBle();

        bool Deserialize(ash::AshBuffer* Buffer);
        bool Cleanup();

        inline ash::AshCRC32Value GetHash()
        {
            return m_Hash;
        }

        NearbyDiscoveredEndpointId GetEndpointId();

        inline nearby_medium_advertisement_ble* GetMedium()
        {
            return m_Medium;
        }

        inline nearby_connection_advertisement_ble* GetConnection()
        {
            return m_Connection;
        }

        inline nearby_share_advertisement* GetShare()
        {
            return m_Share;
        }
    private:
        ash::AshCRC32Value m_Hash;
        nearby_medium_advertisement_ble* m_Medium;
        nearby_connection_advertisement_ble* m_Connection;
        nearby_share_advertisement* m_Share;
    };



} // namespace nearby::client

#endif // !_NEARBYCLIENT_DISCOVERED_HPP
