#ifndef _NEARBYCLIENT_SERVICES_OAUTH_AUTHORIZER_HPP
#define _NEARBYCLIENT_SERVICES_OAUTH_AUTHORIZER_HPP

#include "NearbyClient/Services/OAuth/Token.hpp"
#include <functional>
#include <string>
#include <thread>
#include <httplib.h>

namespace nearby::client::services
{

    class OAuthorizer
    {
    public:
        using dReceivedOAuthToken = std::function<void(OAuthToken Token)>;

        OAuthorizer(dReceivedOAuthToken ReceivedOAuthToken);

        void Start();
        void Stop();
        std::string GetBrowserLoginUrl();
    private:
        void Thread();
    private:
        std::jthread* m_Thread;
        unsigned short m_Port;
        std::string m_State;
        httplib::Server* m_Server;
        std::string m_Code;
        dReceivedOAuthToken m_ReceivedOAuthToken;
    };

} // namespace nearby::client::services

#endif // !_NEARBYCLIENT_SERVICES_OAUTH_AUTHORIZER_HPP
