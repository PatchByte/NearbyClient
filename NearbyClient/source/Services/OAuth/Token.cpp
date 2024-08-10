#include "NearbyClient/Services/OAuth/Token.hpp"
#include <ctime>
#include <vector>

namespace nearby::client::services
{
    bool OAuthToken::ImportFromGoogleTokenResponse(nlohmann::json Data, bool IsRefreshTokenResponse)
    {
        std::vector<std::string> requiredFields = {"access_token", "expires_in", "scope", "token_type", "id_token"};

        if (IsRefreshTokenResponse == false)
        {
            requiredFields.push_back("refresh_token");
        }

        for (std::string currentRequiredField : requiredFields)
        {
            if (Data[currentRequiredField].is_null())
            {
                return false;
            }
        }

        m_AccessToken = Data["access_token"].get<std::string>();
        m_ExpireTimeStamp = time(nullptr) + Data["expires_in"].get<int>();

        if (IsRefreshTokenResponse == false)
        {
            m_RefreshToken = Data["refresh_token"].get<std::string>();
        }

        m_Scope = Data["scope"].get<std::string>();
        m_TokenType = Data["token_type"].get<std::string>();
        m_IdToken = Data["id_token"].get<std::string>();

        return true;
    }

    bool OAuthToken::Import(nlohmann::json Data)
    {
        std::vector<std::string> requiredFields = {"access_token", "refresh_token", "expires_at", "scope", "token_type", "id_token"};

        for (std::string currentRequiredField : requiredFields)
        {
            if (Data[currentRequiredField].is_null())
            {
                return false;
            }
        }

        m_AccessToken = Data["access_token"].get<std::string>();
        m_ExpireTimeStamp = Data["expires_at"].get<unsigned long long>();
        m_RefreshToken = Data["refresh_token"].get<std::string>();
        m_Scope = Data["scope"].get<std::string>();
        m_TokenType = Data["token_type"].get<std::string>();
        m_IdToken = Data["id_token"].get<std::string>();

        return true;
    }

    nlohmann::json OAuthToken::Export()
    {
        nlohmann::json data = {};

        data["access_token"] = m_AccessToken;
        data["expires_at"] = m_ExpireTimeStamp;
        data["refresh_token"] = m_RefreshToken;
        data["scope"] = m_Scope;
        data["token_type"] = m_TokenType;
        data["id_token"] = m_IdToken;

        return std::move(data);
    }

} // namespace nearby::client::services
