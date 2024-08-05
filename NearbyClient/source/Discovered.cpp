#include "NearbyClient/Discovered.hpp"
#include "NearbyProtocols/ConnectionAdvertisement.h"
#include "NearbyProtocols/MediumAdvertisement.h"
#include "NearbyProtocols/ShareAdvertisement.h"
#include "NearbyUtils/Buffer.h"
#include <string.h>

namespace nearby::client
{

    NearbyDiscoveredAdvertisement::NearbyDiscoveredAdvertisement()
        : m_Medium(new nearby_medium_advertisement_ble()), m_Connection(new nearby_connection_advertisement_ble()), m_Share(new nearby_share_advertisement())
    {
        memset(m_Medium, 0, sizeof(*m_Medium));
        memset(m_Connection, 0, sizeof(*m_Connection));
        memset(m_Share, 0, sizeof(*m_Share));
    }

    NearbyDiscoveredAdvertisement::~NearbyDiscoveredAdvertisement()
    {
        this->Reset();
    }

    bool NearbyDiscoveredAdvertisement::Deserialize(void* AdvertisementData, size_t AdvertisementLength)
    {
        nearby_utils_buffer mediumBuffer = nearby_utils_buffer();
        nearby_utils_buffer_initialize(&mediumBuffer, AdvertisementData, AdvertisementLength);

        if (nearby_medium_advertisement_ble_deserialize(m_Medium, &mediumBuffer) == false)
        {
            this->Reset();
            return false;
        }

        nearby_utils_buffer connectionBuffer = nearby_utils_buffer();
        nearby_utils_buffer_initialize(&connectionBuffer, m_Medium->data, m_Medium->data_size);

        nearby_connection_advertisement_ble_deserialize(m_Connection, &connectionBuffer, m_Medium->is_fast_advertisement);

        if (connectionBuffer.has_error_occurred == true)
        {
            this->Reset();
            return false;
        }

        nearby_utils_buffer shareBuffer = nearby_utils_buffer();
        nearby_utils_buffer_initialize(&shareBuffer, m_Connection->endpoint_info_buffer, m_Connection->endpoint_info_length);

        if (nearby_share_advertisement_from_endpoint_info(m_Share, &shareBuffer) == false)
        {
            this->Reset();
            return false;
        }

        return true;
    }

    bool NearbyDiscoveredAdvertisement::Reset()
    {
        if (m_Share)
        {
            nearby_share_advertisement_from_endpoint_info_cleanup(m_Share);

            delete m_Share;
            m_Share = nullptr;
        }

        if (m_Connection)
        {
            nearby_connection_advertisement_ble_deserialize_cleanup(m_Connection);

            delete m_Connection;
            m_Connection = nullptr;
        }

        if (m_Medium)
        {
            nearby_medium_advertisement_ble_deserialize_cleanup(m_Medium);

            delete m_Medium;
            m_Medium = nullptr;
        }

        return true;
    }

} // namespace nearby::client
