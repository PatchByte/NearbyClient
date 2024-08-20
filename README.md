# NearbyClient

## Important Note

This is work in progress

## Bluez doesn't give you access to BLE Scanning?

Do:
```
sudo setcap 'cap_net_raw,cap_net_admin+eip' ./NearbyClient
```

[The StackOverflow Solution](https://stackoverflow.com/a/50652858)
