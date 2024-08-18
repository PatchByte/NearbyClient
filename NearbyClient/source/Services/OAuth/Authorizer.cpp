#include "NearbyClient/Services/OAuth/Authorizer.hpp"
#include "NearbyClient/Services/OAuth/Token.hpp"
#include "NearbyClient/Services/Variables.hpp"
#include "NearbyClient/Utilities/Uri.hpp"
#include "fmt/format.h"
#include "ixwebsocket/IXConnectionState.h"
#include "ixwebsocket/IXHttp.h"
#include "ixwebsocket/IXHttpServer.h"
#include <cstdlib>
#include <future>
#include <iostream>
#include <random>
#include <thread>

namespace nearby::client::services
{
    // Too lazy to code
    // Copied from: https://inversepalindrome.com/blog/how-to-create-a-random-string-in-cpp
    static std::string sfRandomString(std::size_t length)
    {
        static constexpr std::string_view smCharacters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<> distribution(0, smCharacters.size() - 1);

        std::string randomStrng;

        for (std::size_t i = 0; i < length; ++i)
        {
            randomStrng += smCharacters[distribution(generator)];
        }

        return randomStrng;
    }

    OAuthorizer::OAuthorizer(dReceivedOAuthToken ReceivedOAuthToken) : m_Port(-1), m_Thread(nullptr), m_State(), m_ReceivedOAuthToken(ReceivedOAuthToken), m_Code(), m_Server(), m_Running(false)
    {
    }

    void OAuthorizer::Start()
    {
        if (m_Thread)
        {
            return;
        }

        m_Port = 57947;
        m_State = sfRandomString(16);
        m_Code = "";
        m_Thread = new std::jthread([&] { this->Thread(); });
    }

    void OAuthorizer::Stop()
    {
        if (m_Server)
        {
            delete m_Server;
            m_Server = nullptr;
        }

        if (m_Thread)
        {
            if (m_Thread->joinable())
            {
                m_Thread->join();
            }

            delete m_Thread;
            m_Thread = nullptr;
        }
    }

    std::string OAuthorizer::GetBrowserLoginUrl()
    {
        return fmt::format("https://accounts.google.com/o/oauth2/auth/"
                           "oauthchooseaccount?client_id={}&redirect_uri=http%3A%2F%2F127.0.0.1%3A{}%2Fcallback&state="
                           "{}&scope=email%20profile%20https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fnearbysharing-pa%20https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fexperimentsandconfigs%"
                           "20https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fcloudplatformprojects&response_type=code&prompt=consent&access_type=offline&service=lso&o2v=1&ddm=0&flowName=GeneralOAuthFlow",
                           Variables::GetClientId(), m_Port, m_State);
    }

    void OAuthorizer::Thread()
    {
        m_Server = new ix::HttpServer(m_Port, "127.0.0.1");

        // "/callback"

        m_Server->setOnConnectionCallback(
            [this](ix::HttpRequestPtr Request, std::shared_ptr<ix::ConnectionState> ConnectionState) -> ix::HttpResponsePtr
            {
                /*
                std::string code = Request.get_param_value("code");

                if (code.length() > 0)
                {
                    m_Code = code;

                    Response.set_content("<!DOCTYPE html><html><body><h1>Close this now, you can go back into the application.</h1></body></html>", "text/html");

                    std::ignore = std::async(std::launch::async, [](httplib::Server* Server) { Server->stop(); }, m_Server);

                    return;
                }

                Response.set_content("<html><body><h1>Error has occurred.</h1></body></html>", "text/html");
                 */

                ix::HttpResponsePtr response = std::make_shared<ix::HttpResponse>();

                response->headers["content-type"] = "text/html";

                uri parsedUri = uri(std::string("https://127.0.0.1:80") + Request->uri);

                if (parsedUri.get_query_dictionary().contains("code"))
                {
                    response->body = "<!DOCTYPE html><html><body><h1>Close this now, you can go back into the application.</h1></body></html>";

                    m_Code = parsedUri.get_query_dictionary().at("code");

                    std::ignore = std::async(std::launch::async, [this]() { m_Running = false; });
                }
                else
                {
                    response->body = "<html><body><h1>Error has occurred.</h1></body></html>";
                }

                return response;
            });

        m_Running = m_Server->listenAndStart();

        while(m_Running)
        {
            std::this_thread::yield();
        }

        m_Server->stop();

        std::cout << "a" << std::endl;

        OAuthToken token = OAuthToken();

        if (token.RequestTokenFromCode(Variables::GetClientId(), Variables::GetClientSecret(), fmt::format("http://127.0.0.1:{}/callback", m_Port), m_Code) == true)
        {
            m_ReceivedOAuthToken(token);
        }
        else
        {
            printf("[!] Receieved error while oauth.\n");
        }

        delete m_Server;
        m_Server = nullptr;
    }

} // namespace nearby::client::services
