#ifndef _NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP
#define _NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP

#include "Ash/AshCRC32.h"
#include "NearbyClient/DiscoveredAdvertisements.hpp"
#include "NearbyClient/DiscoveredEndpointId.hpp"
#include <chrono>

namespace nearby::client
{

    enum class NearbyDiscoveredEndpointType : unsigned char
    {
        INVALID = 0,
        BLE = 1
    };

    struct NearbyDiscoveredEndpointMetadata
    {
        NearbyDiscoveredEndpointType m_Type;
        NearbyDiscoveredEndpointId m_UniqueId;
        std::string m_Display;
    };

    class NearbyDiscoveredEndpointBase
    {
    public:
        NearbyDiscoveredEndpointBase();
        virtual ~NearbyDiscoveredEndpointBase() = default;

        virtual NearbyDiscoveredEndpointMetadata GetMetadata()
        {
            return m_Metadata;
        }

        // Ping System to check for lost devices.

        inline std::chrono::time_point<std::chrono::system_clock> GetLastLifeSign()
        {
            return m_LastLifeSign;
        }

        void SetReceivedLastLifeSign();
        bool HasLastLifeSignTimeouted(std::chrono::seconds MaxTimeout);

        // Gui Abstraction Layer

        virtual void RenderDebugFrame() {};

    protected:
        NearbyDiscoveredEndpointMetadata m_Metadata;
        std::chrono::time_point<std::chrono::system_clock> m_LastLifeSign;
    };

    class NearbyDiscoveredEndpointBle : public NearbyDiscoveredEndpointBase
    {
    public:
        //! @warning @param[in] Advertisement Is being consumed.
        NearbyDiscoveredEndpointBle(unsigned char* MacAddress, NearbyDiscoveredAdvertisementBle* Advertisement);
        ~NearbyDiscoveredEndpointBle() override;

        void RenderDebugFrame() override;

        // Ble Endpoint Specific Methods

        unsigned char* GetMacAddress()
        {
            return m_MacAddress;
        }

        //! @warning @param[in] Advertisement Is being consumed.
        void SetAdvertisement(NearbyDiscoveredAdvertisementBle* Advertisement, bool FreeOld = true);

        NearbyDiscoveredAdvertisementBle* GetAdvertisement()
        {
            return m_Advertisement;
        }

    private:
        unsigned char m_MacAddress[6];
        NearbyDiscoveredAdvertisementBle* m_Advertisement;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP
