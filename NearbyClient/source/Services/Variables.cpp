#include "NearbyClient/Services/Variables.hpp"
#include "Ash/AshBase64.h"

namespace nearby::client::services
{
    std::string Variables::GetClientId()
    {
        auto bufferDecoded = ash::AshBase64::Decode("NDI3NzI5NTI5MTQtdnRsMWZqZ29yMGg3OWZhYXFkNGpjM3YzbHU0bzRsNGYuYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20=");
        std::string clientId = bufferDecoded->GetAsString();
        delete bufferDecoded;
        return clientId;
    }

    std::string Variables::GetClientSecret()
    {
        auto bufferDecoded = ash::AshBase64::Decode("R09DU1BYLWtBVk9WcE5hbDdOU1pReHkwMjhYRHZyYnUtT3Y=");
        std::string clientSecret = bufferDecoded->GetAsString();
        delete bufferDecoded;
        return clientSecret;
    }
} // namespace nearby::client::services
