#ifndef _NEARBYCLIENT_SERVICES_NEARBYSHARE_HPP
#define _NEARBYCLIENT_SERVICES_NEARBYSHARE_HPP

#include "AshLogger/AshLogger.h"
#include "NearbyClient/Services/OAuth/Token.hpp"
#include "NearbyStorage/Certificate.h"
#include "httplib.h"
#include <vector>

namespace nearby::client::services
{

    class NearbyShare
    {
    public:
        struct ListPublicCertificatesResponse
        {
            bool m_Error;
            std::vector<nearby_storage_public_certificate*> m_Ceritficates;
        };

        NearbyShare(ash::AshLogger& Logger);

        inline NearbyShare& SetToken(OAuthToken Token)
        {
            m_Token = Token;
            return *this;
        }

        ListPublicCertificatesResponse ListPublicCertificates();

    private:
        ash::AshLogger& m_Logger;
        OAuthToken m_Token;
        httplib::SSLClient m_Client;
    };

} // namespace nearby::client::services

#endif // !_NEARBYCLIENT_SERVICES_NEARBYSHARE_HPP
