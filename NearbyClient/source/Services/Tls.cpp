#include "NearbyClient/Services/Tls.hpp"
#include <filesystem>
#include <vector>

namespace nearby::client::services
{

    static std::vector<std::filesystem::path> sCaCertsFiles = {"/etc/ssl/certs/ca-certificates.crt", "/etc/pki/tls/certs/ca-bundle.crt"};

    ix::SocketTLSOptions TLS::sfCreateTlsOptions()
    {
        ix::SocketTLSOptions socketTlsOptions = ix::SocketTLSOptions();

        socketTlsOptions.tls = true;

#if linux
        for (std::filesystem::path currentPath : sCaCertsFiles)
        {
            if (std::filesystem::exists(currentPath))
            {
                socketTlsOptions.caFile = currentPath.string();
                break;
            }
        }
#endif

        return socketTlsOptions;
    }

} // namespace nearby::client::services
