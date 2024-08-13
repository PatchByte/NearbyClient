#include "NearbyClient/Main.hpp"
#include "NearbyClient/Client.hpp"
#include "curl/curl.h"

int main(int ArgCount, char** ArgVector)
{
    curl_global_init(CURL_GLOBAL_ALL);

    nearby::client::NearbyClient client = nearby::client::NearbyClient();
    client.Run();

    curl_global_cleanup();

    return 0;
}
