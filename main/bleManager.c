// #include "bleManager.h"
// #include "esp_log.h"
// #include <string.h>

// static const char* TAG = "BT_MANAGER";

// static bluetooth_manager_config_t bt_config = {
//     .device_name = "ESP32 Device",
//     .is_advertising = false,
//     .is_connected = false
// };

// esp_err_t bluetooth_manager_init(const char* device_name) {
//     esp_err_t ret;

//     // Ensure we're using BLE only mode
//     esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    
//     // Initialize Bluetooth controller
//     ret = esp_bt_controller_init(&bt_cfg);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Bluetooth controller init failed: %s", esp_err_to_name(ret));
//         return ret;
//     }

//     // Enable BLE only mode
//     ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
//         return ret;
//     }

//     // Copy device name
//     strncpy(bt_config.device_name, device_name, BT_DEVICE_NAME_MAX_LEN - 1);
//     bt_config.device_name[BT_DEVICE_NAME_MAX_LEN - 1] = '\0';

//     ESP_LOGI(TAG, "Bluetooth initialized successfully");
//     return ESP_OK;
// }

// esp_err_t bluetooth_manager_start_advertising(void) {
//     // Configure advertising parameters
//     esp_ble_adv_params_t adv_params = {
//         .adv_int_min        = 0x20,
//         .adv_int_max        = 0x40,
//         .adv_type           = ADV_TYPE_IND,
//         .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
//         .channel_map        = ADV_CHNL_ALL,
//         .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
//     };

//     // Set device name
//     esp_ble_gap_set_device_name(bt_config.device_name);

//     // Start advertising
//     esp_err_t ret = esp_ble_gap_start_advertising(&adv_params);
//     if (ret == ESP_OK) {
//         bt_config.is_advertising = true;
//         ESP_LOGI(TAG, "Advertising started");
//     } else {
//         ESP_LOGE(TAG, "Failed to start advertising: %s", esp_err_to_name(ret));
//     }

//     return ret;
// }

// esp_err_t bluetooth_manager_stop_advertising(void) {
//     esp_err_t ret = esp_ble_gap_stop_advertising();
//     if (ret == ESP_OK) {
//         bt_config.is_advertising = false;
//         ESP_LOGI(TAG, "Advertising stopped");
//     } else {
//         ESP_LOGE(TAG, "Failed to stop advertising: %s", esp_err_to_name(ret));
//     }

//     return ret;
// }

// void bluetooth_manager_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
//     switch (event) {
//         case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
//             if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
//                 ESP_LOGE(TAG, "Advertising start failed");
//             }
//             break;

//         case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
//             if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
//                 ESP_LOGE(TAG, "Advertising stop failed");
//             }
//             break;

//         default:
//             break;
//     }
// }

// void bluetooth_manager_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
//     switch (event) {
//         case ESP_GATTS_REG_EVT:
//             ESP_LOGI(TAG, "GATT server registered");
//             break;

//         case ESP_GATTS_CONNECT_EVT:
//             bt_config.is_connected = true;
//             ESP_LOGI(TAG, "BLE device connected");
//             break;

//         case ESP_GATTS_DISCONNECT_EVT:
//             bt_config.is_connected = false;
//             ESP_LOGI(TAG, "BLE device disconnected");
//             bluetooth_manager_start_advertising();
//             break;

//         default:
//             break;
//     }
// }

// bool bluetooth_manager_is_connected(void) {
//     return bt_config.is_connected;
// }

// const char* bluetooth_manager_get_device_name(void) {
//     return bt_config.device_name;
// }