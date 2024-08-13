#ifndef _NEARBYCLIENT_UTILITIES_CURL_HPP
#define _NEARBYCLIENT_UTILITIES_CURL_HPP

#include "Ash/AshBuffer.h"
#include <map>
#include <string>
namespace nearby::client::utilities
{

    class Curl
    {
    public:
        enum Methods
        {
            GET = 0,
            POST = 1
        };

        using dHeaders = std::vector<std::string>;

        struct RequestParameters
        {
            std::string m_Url;
            dHeaders m_Headers;
            std::string m_ContentType;
            ash::AshBuffer m_ContentData;
        };

        struct ResponseParameters
        {
            int m_StatusCode;
            ash::AshBuffer m_Content;
        };

        static ResponseParameters Perform(RequestParameters Request);
    };

}

#endif // !_NEARBYCLIENT_UTILITIES_CURL_HPP
