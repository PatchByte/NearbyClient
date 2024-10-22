#ifndef _NEARBYCLIENT_SEVICES_OAUTH_TOKEN_HPP
#define _NEARBYCLIENT_SEVICES_OAUTH_TOKEN_HPP

#include "nlohmann/json_fwd.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

namespace nearby::client::services
{

    class OAuthToken
    {
    public:
        inline OAuthToken() : m_IdToken(), m_Scope(), m_RefreshToken(), m_AccessToken(), m_ExpireTimeStamp()
        {
        }

        inline OAuthToken(std::string AccessToken, std::string RefreshToken, uint64_t ExpireTimeStamp, std::string Scope, std::string TokenType, std::string IdToken)
            : m_AccessToken(AccessToken), m_RefreshToken(RefreshToken), m_ExpireTimeStamp(ExpireTimeStamp), m_Scope(Scope), m_TokenType(TokenType), m_IdToken(IdToken)
        {
        }

        std::string GetAccessToken()
        {
            return m_AccessToken;
        }

        std::string GetRefreshToken()
        {
            return m_RefreshToken;
        }

        uint64_t GetExpireTimeStamp()
        {
            return m_ExpireTimeStamp;
        }

        std::string GetScope()
        {
            return m_Scope;
        }

        std::string GetTokenType()
        {
            return m_TokenType;
        }

        std::string GetIdToken()
        {
            return m_IdToken;
        }

        bool IsExpired();
        uint64_t GetSecondsWillExpireIn();

        // This is a method to import the contents of the `https://oauth2.googleapis.com/token`
        bool ImportFromGoogleTokenResponse(nlohmann::json Data, bool IsRefreshTokenResponse = false);
        // This is a custom method to import
        bool Import(nlohmann::json Data);
        // This is a custom method to export
        nlohmann::json Export();

        bool RequestTokenFromCode(std::string ClientId, std::string ClientSecret, std::string RedirectUri, std::string Code);
        bool RefreshToken(std::string ClientId, std::string ClientSecret);
    private:
        std::string m_AccessToken;
        std::string m_RefreshToken;
        uint64_t m_ExpireTimeStamp;
        std::string m_Scope;
        std::string m_TokenType;
        std::string m_IdToken;
    };

} // namespace nearby::client::services

#endif // !_NEARBYCLIENT_SEVICES_OAUTH_TOKEN_HPP
