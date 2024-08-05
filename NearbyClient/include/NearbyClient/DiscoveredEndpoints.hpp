#ifndef _NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP
#define _NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP

#include "NearbyClient/DiscoveredAdvertisements.hpp"
#include <chrono>

namespace nearby::client
{

    enum class NearbyDiscoveredEndpointType : unsigned char
    {
        INVALID = 0,
        BLE = 1
    };

    class NearbyDiscoveredEndpointBase
    {
    public:
        NearbyDiscoveredEndpointBase();
        virtual ~NearbyDiscoveredEndpointBase() = default;

        virtual NearbyDiscoveredEndpointType GetType()
        {
            return NearbyDiscoveredEndpointType::INVALID;
        }

        virtual void RenderDebugFrame() = 0;

        inline std::chrono::seconds GetLastPing()
        {
            return m_LastPing;
        }

        void DoPing();
        bool IsPingTimeout(std::chrono::seconds MaxTimeout);

    protected:
        std::chrono::seconds m_LastPing;
    };

    class NearbyDiscoveredEndpointBle : public NearbyDiscoveredEndpointBase
    {
    public:
        //! @warning @param[in] Advertisement Is being consumed.
        NearbyDiscoveredEndpointBle(NearbyDiscoveredAdvertisementBle* Advertisement);
        ~NearbyDiscoveredEndpointBle() override;

        NearbyDiscoveredEndpointType GetType() override
        {
            return NearbyDiscoveredEndpointType::BLE;
        }

        void RenderDebugFrame() override;

        // Bluetooth Specific Things

        inline unsigned char* GetMacAddress()
        {
            return m_MacAddress;
        }

        inline NearbyDiscoveredAdvertisementBle* GetAdvertisement()
        {
            return m_Advertisement;
        }

    private:
        unsigned char m_MacAddress[6];
        NearbyDiscoveredAdvertisementBle* m_Advertisement;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP
