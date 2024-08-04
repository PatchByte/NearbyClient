#include "NearbyClient/Main.hpp"
#include "NearbyClient/Client.hpp"

int main(int ArgCount, char** ArgVector)
{
    nearby::client::NearbyClient client = nearby::client::NearbyClient();

    client.Run();

    return 0;
}
