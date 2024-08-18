#ifndef _NEARBYCLIENT_SERVICES_TLS_HPP
#define _NEARBYCLIENT_SERVICES_TLS_HPP

#include "ixwebsocket/IXSocketTLSOptions.h"
namespace nearby::client::services
{

    class TLS
    {
    public:
        static ix::SocketTLSOptions sfCreateTlsOptions();
    };

}

#endif // !_NEARBYCLIENT_SERVICES_TLS_HPP
