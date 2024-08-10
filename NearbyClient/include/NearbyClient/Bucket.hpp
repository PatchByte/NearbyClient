#ifndef _NEARBYCLIENT_BUCKET_HPP
#define _NEARBYCLIENT_BUCKET_HPP

#include "NearbyClient/Services/OAuth/Token.hpp"
#include <filesystem>

namespace nearby::client
{

    class Bucket
    {
    public:
        Bucket();

        void SaveToFile(std::filesystem::path Path);
        bool LoadFromFile(std::filesystem::path Path);

        services::OAuthToken& GetToken()
        {
            return m_Token;
        }

        Bucket& SetToken(services::OAuthToken Token)
        {
            m_Token = Token;
            m_HasToken = true;
            return *this;
        }

        bool HasToken()
        {
            return m_HasToken;
        }

        Bucket& SetHasToken(bool HasToken)
        {
            m_HasToken = HasToken;
            return *this;
        }

    private:
        services::OAuthToken m_Token;
        bool m_HasToken;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_BUCKET_HPP
