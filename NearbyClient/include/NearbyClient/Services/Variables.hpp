#ifndef _NEARBYCLIENT_SERVICES_VARIABLES_HPP
#define _NEARBYCLIENT_SERVICES_VARIABLES_HPP

#include <string>

namespace nearby::client::services
{

    /*
     * Hello Mr. Google Employee if you change this variable because of this repo, you are an asshole.
     * Google pretends to love open source but in reality you are just another soulless person sucked up in a corporation just "trying" to make money.
     */
    class Variables
    {
    public:
        static std::string GetClientId();
        static std::string GetClientSecret();
    };

} // namespace nearby::client::services

#endif // !_NEARBYCLIENT_SERVICES_VARIABLES_HPP
