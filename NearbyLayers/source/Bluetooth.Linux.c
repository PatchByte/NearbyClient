#include "NearbyLayers/Bluetooth.h"
#include <unistd.h>
#if linux
#include <complex.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// useful helpers

#define zero_memory(x) memset(&x, 0, sizeof(x));

// hci helper

struct hci_request ble_hci_request(uint16_t ocf, int clen, void* status, void* cparam)
{
    struct hci_request rq;
    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = ocf;
    rq.cparam = cparam;
    rq.clen = clen;
    rq.rparam = status;
    rq.rlen = 1;
    return rq;
}

// Backend Structure

struct nearby_layer_bluetooth
{
    pthread_t scanning_thread;
    bool scanning_thread_should_shutdown;
    bool is_scanning;

    // hci
    int hci_route;
    int hci_device;
};

// Low-Level Implementation

void* nearby_layer_bluetooth_thread_entry(void* parameter);
void nearby_layer_bluetooth_thread(nearby_layer_bluetooth_t* instance);

void* nearby_layer_bluetooth_thread_entry(void* parameter)
{
    nearby_layer_bluetooth_t* instance = parameter;

    instance->is_scanning = true;

    printf("Start\n");

    nearby_layer_bluetooth_thread(instance);

    printf("Stop\n");

    hci_close_dev(instance->hci_device);

    instance->scanning_thread_should_shutdown = false;
    instance->is_scanning = false;

    pthread_exit(NULL);
}

void nearby_layer_bluetooth_thread(nearby_layer_bluetooth_t* instance)
{
    instance->hci_route = hci_get_route(NULL);

    if (instance->hci_route < 0)
    {
        printf("[!] hci_get_route invalid route.\n");
        return;
    }

    instance->hci_device = hci_open_dev(instance->hci_route);

    if (instance->hci_device < 0)
    {
        printf("[!] hci_open_dev invalid deivce.\n");
        return;
    }

    int ret = 0, status = 0;

    // Set BLE Scan Parameters

    le_set_scan_parameters_cp ble_scan_parameters;
    zero_memory(ble_scan_parameters);

    ble_scan_parameters.type = 0x00;
    ble_scan_parameters.interval = htobs(0x0010);
    ble_scan_parameters.window = htobs(0x0010);
    ble_scan_parameters.own_bdaddr_type = LE_PUBLIC_ADDRESS; // Public Device Address (default).
    ble_scan_parameters.filter = 0x00;                       // Accept all.

    struct hci_request hci_ble_scan_parameters_request = ble_hci_request(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &status, &ble_scan_parameters);
    ret = hci_send_req(instance->hci_device, &hci_ble_scan_parameters_request, 1000);

    if (ret < 0)
    {
        printf("[!] hci_send_req(le_set_scan_parameters_cp) failed, maybe your system is blocking ble requests. Please take a look at the readme to solve this problem!\n");
        return;
    }

    // Set BLE events report mask

    le_set_event_mask_cp ble_event_mask;
    zero_memory(ble_event_mask);

    for (int i = 0; i < 8; i++)
    {
        ble_event_mask.mask[i] = 0xFF;
    }

    struct hci_request hci_ble_event_mask = ble_hci_request(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &status, &ble_event_mask);
    ret = hci_send_req(instance->hci_device, &hci_ble_scan_parameters_request, 1000);

    if (ret < 0)
    {
        printf("[!] hci_send_req(le_set_event_mask_cp) failed\n");
        return;
    }

    // Enable scanning

    le_set_scan_enable_cp ble_scan;
    memset(&ble_scan, 0, sizeof(ble_scan));
    ble_scan.enable = 0x01;     // Enable flag.
    ble_scan.filter_dup = 0x00; // Filtering disabled.

    struct hci_request hci_ble_scan = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &ble_scan);

    ret = hci_send_req(instance->hci_device, &hci_ble_scan, 1000);

    if (ret < 0)
    {
        printf("[!] hci_send_req(hci_request) failed\n");
        return;
    }

    // Set Results

    struct hci_filter nf;
    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(instance->hci_device, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0)
    {
        printf("[!] setsockopt failed.\n");
        return;
    }

    {
        uint8_t current_hci_event_buffer[HCI_MAX_EVENT_SIZE];
        evt_le_meta_event* current_hci_meta_event;

        while (instance->is_scanning == true)
        {
            if (instance->scanning_thread_should_shutdown == true)
            {
                break;
            }

            zero_memory(current_hci_event_buffer);

            int current_hci_event_length_read = read(instance->hci_device, current_hci_event_buffer, sizeof(current_hci_event_buffer));
            if (current_hci_event_length_read >= HCI_EVENT_HDR_SIZE)
            {
                current_hci_meta_event = (evt_le_meta_event*)(current_hci_event_buffer + HCI_EVENT_HDR_SIZE + 1);

                if (current_hci_meta_event->subevent == EVT_LE_ADVERTISING_REPORT)
                {
                    size_t current_advertising_report_count = current_hci_meta_event->data[0];
                    void* current_advertising_report_position = current_hci_meta_event->data + 1;
                    while (current_advertising_report_count--)
                    {
                        le_advertising_info* current_advertising_info = (le_advertising_info*)current_advertising_report_position;
                        char addr[18];
                        ba2str(&(current_advertising_info->bdaddr), addr);
                        printf("%s - RSSI %d\n", addr, (char)current_advertising_info->data[current_advertising_info->length]);
                        current_advertising_report_position = current_advertising_info->data + current_advertising_info->length + 2;
                    }
                }
            }

            sched_yield();
        }
    }
}

// High-Level Implementation

nearby_layer_bluetooth_t* nearby_layer_bluetooth_create()
{
    nearby_layer_bluetooth_t* instance = malloc(sizeof(nearby_layer_bluetooth_t));

    instance->scanning_thread = 0;
    instance->is_scanning = false;
    instance->scanning_thread_should_shutdown = false;

    return instance;
}

bool nearby_layer_bluetooth_start_scanning(nearby_layer_bluetooth_t* instance)
{
    if (instance->scanning_thread)
    {
        return false;
    }

    instance->is_scanning = true;
    instance->scanning_thread_should_shutdown = false;

    if (pthread_create(&instance->scanning_thread, NULL, nearby_layer_bluetooth_thread_entry, instance) != 0)
    {
        instance->is_scanning = false;
        return false;
    }

    return true;
}

bool nearby_layer_bluetooth_stop_scanning(nearby_layer_bluetooth_t* instance)
{
    if (instance->scanning_thread)
    {
        instance->scanning_thread_should_shutdown = true;

        pthread_join(instance->scanning_thread, NULL);

        instance->scanning_thread = 0;

        return true;
    }

    return false;
}

void nearby_layer_bluetooth_destroy(nearby_layer_bluetooth_t* instance)
{
    nearby_layer_bluetooth_stop_scanning(instance);

    free(instance);
}

bool nearby_layer_bluetooth_is_running(nearby_layer_bluetooth_t* instance)
{
    return instance->is_scanning;
}

#endif
