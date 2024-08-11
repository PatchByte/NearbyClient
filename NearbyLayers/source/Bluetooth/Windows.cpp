#ifdef _WIN32
#include <Windows.h>
#include <cstdint>

#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>

#include "NearbyLayers/Bluetooth.h"

using namespace winrt;

using namespace Windows::Devices;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Storage::Streams;

struct nearby_layer_bluetooth
{
    BluetoothLEAdvertisementWatcher watcher;
};

nearby_layer_bluetooth_t* nearby_layer_bluetooth_create()
{
    nearby_layer_bluetooth_t* instance = new nearby_layer_bluetooth_t();

    return instance;
}

bool nearby_layer_bluetooth_start_scanning(nearby_layer_bluetooth_t* instance)
{
    instance->watcher.Start();
    return true;
}

bool nearby_layer_bluetooth_stop_scanning(nearby_layer_bluetooth_t* instance)
{
    instance->watcher.Stop();
    return true;
}

void nearby_layer_bluetooth_destroy(nearby_layer_bluetooth_t* instance)
{
    delete instance;
    instance = nullptr;
}

bool nearby_layer_bluetooth_is_scanning(nearby_layer_bluetooth_t* instance)
{
    return instance->watcher.Status() == BluetoothLEAdvertisementWatcherStatus::Started;
}

void nearby_layer_bluetooth_set_discovered_advertisement_handler(nearby_layer_bluetooth_t* instance, nearby_layer_bluetooth_discovered_advertisement_handler_t handler, void* handler_user_parameter)
{
    instance->watcher.Received(
        [handler, handler_user_parameter](BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs args)
        {
            for (auto section : args.Advertisement().DataSections())
            {
                IBuffer buffer = section.Data();

                if (!(buffer.data()[0] == 0xf3 && buffer.data()[1] == 0xfe))
                    continue;

                uint64_t macAddress = args.BluetoothAddress();
                handler((uint8_t*)(&macAddress) + 2, args.BluetoothAddressType() == BluetoothAddressType::Random, buffer.data() + 2, buffer.Length() - 2, handler_user_parameter);

                return;
            }
        });
}

#endif
