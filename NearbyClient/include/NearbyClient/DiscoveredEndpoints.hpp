#ifndef _NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP
#define _NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP

#include "Ash/AshCRC32.h"
#include "NearbyClient/DiscoveredAdvertisements.hpp"
#include <chrono>

namespace nearby::client
{

    enum class NearbyDiscoveredEndpointType : unsigned char
    {
        INVALID = 0,
        BLE = 1
    };

    union NearbyDiscoveredEndpointUniqueId
    {
        unsigned long long m_Raw;

        struct
        {
            NearbyDiscoveredEndpointType m_Type;

            union
            {
                struct
                {
                    unsigned char m_Mac[6];
                } m_Ble;
            };
        } m_Build;
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

        virtual NearbyDiscoveredEndpointUniqueId GetUniqueId()
        {
            return m_UniqueId;
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
        NearbyDiscoveredEndpointUniqueId m_UniqueId;
    };

    class NearbyDiscoveredEndpointBle : public NearbyDiscoveredEndpointBase
    {
    public:
        //! @warning @param[in] Advertisement Is being consumed.
        NearbyDiscoveredEndpointBle(unsigned char* MacAddress, NearbyDiscoveredAdvertisementBle* Advertisement);
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

        static NearbyDiscoveredEndpointUniqueId sfMakeUniqueId(unsigned char* MacAddress);

    private:
        unsigned char m_MacAddress[6];
        NearbyDiscoveredAdvertisementBle* m_Advertisement;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_DISCOVEREDENDPOINTS_HPP
