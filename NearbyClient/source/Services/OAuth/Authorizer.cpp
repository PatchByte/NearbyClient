#include "NearbyClient/Services/OAuth/Authorizer.hpp"
#include "NearbyClient/Services/OAuth/Token.hpp"
#include "NearbyClient/Services/Variables.hpp"
#include "fmt/format.h"
#include <cstdlib>
#include <future>
#include <httplib.h>
#include <random>
#include <thread>

namespace nearby::client::services
{
    // Too lazy to code
    // Copied from: https://inversepalindrome.com/blog/how-to-create-a-random-string-in-cpp
    std::string sfRandomString(std::size_t length)
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

    OAuthorizer::OAuthorizer(dReceivedOAuthToken ReceivedOAuthToken) : m_Port(-1), m_Thread(nullptr), m_State(), m_ReceivedOAuthToken(ReceivedOAuthToken), m_Code()
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
            if (m_Server->is_running())
            {
                m_Server->stop();
            }
            else
            {
                delete m_Server;
                m_Server = nullptr;
            }
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
                           Variables::smClientId, m_Port, m_State);
    }

    void OAuthorizer::Thread()
    {
        m_Server = new httplib::Server();

        m_Server->Get("/callback",
                      [this](const httplib::Request& Request, httplib::Response& Response) -> void
                      {
                          std::string code = Request.get_param_value("code");

                          if (code.length() > 0)
                          {
                              m_Code = code;

                              Response.set_content("<!DOCTYPE html><html><body><h1>Close this now, you can go back into the application.</h1></body></html>", "text/html");

                              std::ignore = std::async(std::launch::async, [](httplib::Server* Server) { Server->stop(); }, m_Server);

                              return;
                          }

                          Response.set_content("<html><body><h1>Error has occurred.</h1></body></html>", "text/html");
                      });

        m_Server->listen("127.0.0.1", m_Port);

        OAuthToken token = OAuthToken();

        if(token.RequestTokenFromCode(std::string(Variables::smClientId), std::string(Variables::smClientSecret), fmt::format("http://127.0.0.1:{}/callback", m_Port), m_Code) == true)
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