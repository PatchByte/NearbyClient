#ifndef _NEARBYCLIENT_DISCOVEREDENDPOINTID_HPP
#define _NEARBYCLIENT_DISCOVEREDENDPOINTID_HPP

namespace nearby::client
{

    using NearbyDiscoveredEndpointId = unsigned long long;

    class NearbyDiscoveredEndpointIdUtil
    {
    public:
        static NearbyDiscoveredEndpointId sfMakeBleUUID(char* EndpointId);
    };

}

#endif // !_NEARBYCLIENT_DISCOVEREDENDPOINTID_HPP
