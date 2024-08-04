#ifndef _NEARBYLAYER_NEARBYLAYERBLUETOOTH_HPP
#define _NEARBYLAYER_NEARBYLAYERBLUETOOTH_HPP

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct nearby_layer_bluetooth;
    typedef struct nearby_layer_bluetooth nearby_layer_bluetooth_t;

    nearby_layer_bluetooth_t* nearby_layer_bluetooth_create();
    bool nearby_layer_bluetooth_start_scanning(nearby_layer_bluetooth_t* instance);
    bool nearby_layer_bluetooth_stop_scanning(nearby_layer_bluetooth_t* instance);
    void nearby_layer_bluetooth_destroy(nearby_layer_bluetooth_t* instance);
    bool nearby_layer_bluetooth_is_running(nearby_layer_bluetooth_t* instance);

#ifdef __cplusplus
}
#endif

#endif
