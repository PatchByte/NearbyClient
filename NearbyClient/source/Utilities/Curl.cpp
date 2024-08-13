#include "NearbyClient/Utilities/Curl.hpp"
#include "curl/curl.h"
#include <curl/easy.h>

namespace nearby::client::utilities
{

    Curl::ResponseParameters Curl::Perform(Curl::RequestParameters Request)
    {
        ::CURL* handle = curl_easy_init();

        curl_easy_setopt(handle, CURLOPT_URL, Request.m_Url.c_str());
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, Request.m_ContentData.GetPointer());
        curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, Request.m_ContentData.GetSize());
        curl_easy_setopt(handle, CURLOPT_POST, 1L);

        curl_slist* headerList = nullptr;

        for(auto currentHeaderIterator : Request.m_Headers)
        {
            headerList = curl_slist_append(headerList, currentHeaderIterator.c_str());
        }

        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headerList);

        curl_easy_perform(handle);
    }

} // namespace nearby::client::utilities
