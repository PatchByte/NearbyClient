./BuildAndConfigure.sh
sudo setcap 'cap_net_raw,cap_net_admin+eip' ./bin/NearbyClient
cd bin
./NearbyClient
