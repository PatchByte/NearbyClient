#include "NearbyClient/DiscoveredEndpointId.hpp"
#include <cstring>

namespace nearby::client
{

    enum class NearbyDiscoveredEndpointIdType : unsigned char
    {
        INVALID = 0,
        BLE = 1
    };

    union NearbyDiscoveredEndpointIdBuild
    {
        unsigned long long m_Raw;

        struct
        {
            NearbyDiscoveredEndpointIdType m_Type;

            union
            {
                struct
                {
                    char m_EndpointId[4];
                } m_Ble;
            };
        } m_Build;
    };

    NearbyDiscoveredEndpointId NearbyDiscoveredEndpointIdUtil::sfMakeBleUUID(char* EndpointId)
    {
        NearbyDiscoveredEndpointIdBuild build;

        build.m_Build.m_Type = NearbyDiscoveredEndpointIdType::BLE;
        memcpy(build.m_Build.m_Ble.m_EndpointId, EndpointId, 4);

        return build.m_Raw;
    }
} // namespace nearby::client
