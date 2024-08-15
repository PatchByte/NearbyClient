#include "NearbyClient/Services/OAuth/Token.hpp"
#include "ixwebsocket/IXHttp.h"
#include "ixwebsocket/IXHttpClient.h"
#include "ixwebsocket/IXSocketTLSOptions.h"
#include "nlohmann/json_fwd.hpp"
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>

namespace nearby::client::services
{
    static ix::SocketTLSOptions sfCreateTlsOptions()
    {
        ix::SocketTLSOptions socketTlsOptions = ix::SocketTLSOptions();

        socketTlsOptions.tls = true;
        socketTlsOptions.caFile = "/etc/pki/tls/certs/ca-bundle.crt";

        return socketTlsOptions;
    }

    bool OAuthToken::IsExpired()
    {
        return time(nullptr) > m_ExpireTimeStamp;
    }

    uint64_t OAuthToken::GetSecondsWillExpireIn()
    {
        if (IsExpired())
        {
            return 0;
        }

        return m_ExpireTimeStamp - time(nullptr);
    }

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

    bool OAuthToken::RequestTokenFromCode(std::string ClientId, std::string ClientSecret, std::string RedirectUri, std::string Code)
    {
        std::string url = "https://oauth2.googleapis.com/token";

        nlohmann::json data = {};

        data["client_id"] = ClientId;
        data["client_secret"] = ClientSecret;
        data["redirect_uri"] = RedirectUri;
        data["grant_type"] = "authorization_code";
        data["code"] = Code;

        ix::HttpClient client = ix::HttpClient();

        client.setTLSOptions(sfCreateTlsOptions());

        ix::HttpRequestArgsPtr clientArgs = client.createRequest();

        clientArgs->extraHeaders["Content-Type"] = "application/json";
        clientArgs->body = data.dump();

        auto res = client.post(url, clientArgs->body, clientArgs);

        std::cout << res->body << std::endl;
        std::cout << res->errorMsg << std::endl;

        if (res->statusCode != 200)
        {
            return false;
        }

        return this->ImportFromGoogleTokenResponse(nlohmann::json::parse(res->body), false);
    }

    bool OAuthToken::RefreshToken(std::string ClientId, std::string ClientSecret)
    {
        std::string url = "https://oauth2.googleapis.com/token";

        nlohmann::json data = {};

        data["client_id"] = ClientId;
        data["client_secret"] = ClientSecret;
        data["grant_type"] = "refresh_token";
        data["refresh_token"] = m_RefreshToken;

        std::string dataSerialized = data.dump();

        ix::HttpClient client = ix::HttpClient();
        ix::HttpRequestArgsPtr clientArgs = client.createRequest();

        clientArgs->extraHeaders["Content-Type"] = "application/json";

        auto res = client.post(url, data.dump(), clientArgs);

        if (res->statusCode != 200)
        {
            return false;
        }

        return this->ImportFromGoogleTokenResponse(nlohmann::json::parse(res->body), true);
    }

} // namespace nearby::client::services
