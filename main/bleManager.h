#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include "esp_system.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_err.h"

#define BT_DEVICE_NAME_MAX_LEN 32

typedef struct {
    char device_name[BT_DEVICE_NAME_MAX_LEN];
    bool is_advertising;
    bool is_connected;
} bluetooth_manager_config_t;

// Initialization functions
esp_err_t bluetooth_manager_init(const char* device_name);
esp_err_t bluetooth_manager_start_advertising(void);
esp_err_t bluetooth_manager_stop_advertising(void);

// Event handlers
void bluetooth_manager_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void bluetooth_manager_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

// Utility functions
bool bluetooth_manager_is_connected(void);
const char* bluetooth_manager_get_device_name(void);

#endif // BLUETOOTH_MANAGER_H