#ifndef _NEARBYLAYER_NEARBYLAYERBLUETOOTH_HPP
#define _NEARBYLAYER_NEARBYLAYERBLUETOOTH_HPP

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct nearby_layer_bluetooth;
    typedef struct nearby_layer_bluetooth nearby_layer_bluetooth_t;

    /*!
     * @brief The corresponding handler handling a discovered nearby share advertisement
     * @warning This handler blocks the scanning thread.
     *
     * @param[in] mac_address This is the mac address as a 6 byte array
     * @param[in] is_random_mac_address
     * @param[in] advertisement_data This is the actual service advertisement data. (After the f3 fe)
     * @param[in] advertisement_length
     */
    typedef void (*nearby_layer_bluetooth_discovered_advertisement_handler_t)(unsigned char* mac_address, bool is_random_mac_address, unsigned char* advertisement_data,
                                                                              unsigned short advertisement_length);

    nearby_layer_bluetooth_t* nearby_layer_bluetooth_create();
    bool nearby_layer_bluetooth_start_scanning(nearby_layer_bluetooth_t* instance);
    bool nearby_layer_bluetooth_stop_scanning(nearby_layer_bluetooth_t* instance);
    void nearby_layer_bluetooth_destroy(nearby_layer_bluetooth_t* instance);
    bool nearby_layer_bluetooth_is_running(nearby_layer_bluetooth_t* instance);
    void nearby_layer_bluetooth_set_discovered_advertisement_handler(nearby_layer_bluetooth_t* instance, nearby_layer_bluetooth_discovered_advertisement_handler_t handler);

#ifdef __cplusplus
}
#endif

#endif
