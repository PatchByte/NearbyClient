#ifndef _NEARBYCLIENT_SERVICES_NEARBYSHARE_HPP
#define _NEARBYCLIENT_SERVICES_NEARBYSHARE_HPP

#include <string_view>

namespace nearby::client::services
{

    /*
     * Hello Mr. Google Employee if you change this variable because of this repo, you are an asshole.
     * Google pretends to love open source but in reality you are just another soulless person sucked up by a corpo "trying" to make money.
     * If you change or try to hide these variables we are going to war.
     */
    class Variables
    {
    public:
        static constexpr std::string_view smClientId = "42772952914-vtl1fjgor0h79faaqd4jc3v3lu4o4l4f.apps.googleusercontent.com";
        static constexpr std::string_view smClientSecret = "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";
    };

} // namespace nearby::client::services

#endif // !_NEARBYCLIENT_SERVICES_NEARBYSHARE_HPP
