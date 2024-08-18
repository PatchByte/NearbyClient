#include "NearbyClient/Services/Tls.hpp"

namespace nearby::client::services
{

    ix::SocketTLSOptions TLS::sfCreateTlsOptions()
    {
        ix::SocketTLSOptions socketTlsOptions = ix::SocketTLSOptions();

        socketTlsOptions.tls = true;

        #if linux
        socketTlsOptions.caFile = "/etc/pki/tls/certs/ca-bundle.crt";
        #endif

        return socketTlsOptions;
    }

} // namespace nearby::client::services
