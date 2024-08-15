#include "NearbyClient/Main.hpp"
#include "NearbyClient/Client.hpp"
#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXSocketMbedTLS.h"
#include "ixwebsocket/IXSocketTLSOptions.h"

int main(int ArgCount, char** ArgVector)
{
    ix::initNetSystem();

    nearby::client::NearbyClient client = nearby::client::NearbyClient();
    client.Run();

    ix::uninitNetSystem();

    return 0;
}
