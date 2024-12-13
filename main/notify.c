#include "notify.h"
#include "bleManager.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include <string.h>

// Reduce static allocations and use const where possible
static const char* const TAG = "NOTIFICATION_SERVICE";

// Service and Characteristic UUIDs - move to .rodata
static const uint16_t GATTS_SERVICE_UUID   = 0x00FF;
static const uint16_t GATTS_CHAR_UUID      = 0xFF01;

// Minimize global state tracking
typedef struct {
    bool is_connected;
    uint16_t conn_id;
    esp_gatt_if_t gatts_interface;
    uint16_t notification_handle;
} NotificationContext;

// Use static allocation for context to reduce dynamic memory usage
static NotificationContext s_notification_ctx = {
    .is_connected = false,
    .conn_id = 0,
    .gatts_interface = 0,
    .notification_handle = 0
};

// Characteristic configuration - use const and move to .rodata
static const uint16_t primary_service_uuid = GATTS_SERVICE_UUID;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_notification_uuid = GATTS_CHAR_UUID;
static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;

// Optimize event handler - minimize IRAM usage
void notification_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(TAG, "GATT service registered");
            break;

        case ESP_GATTS_CONNECT_EVT:
            s_notification_ctx.is_connected = true;
            s_notification_ctx.conn_id = param->connect.conn_id;
            s_notification_ctx.gatts_interface = gatts_if;
            ESP_LOGI(TAG, "Device connected");
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            s_notification_ctx.is_connected = false;
            ESP_LOGI(TAG, "Device disconnected");
            // Restart advertising using the Bluetooth manager
            bluetooth_manager_start_advertising();
            break;

        default:
            break;
    }
}

esp_err_t notification_init(void) {
    esp_err_t ret;

    // Register GATTS callback using the custom handler
    ret = esp_ble_gatts_register_callback(notification_gatts_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GATTS callback registration failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create GATT service - use stack allocation
    esp_gatts_attr_db_t gatt_db[3] = {
        // Service Declaration
        [0] = {
            .attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
            .att_desc = {
                .uuid_length = ESP_UUID_LEN_16,
                .uuid_p = (uint8_t*)&primary_service_uuid,
                .perm = ESP_GATT_PERM_READ,
                .max_length = sizeof(uint16_t),
                .length = sizeof(uint16_t)
            }
        },
        // Characteristic Declaration
        [1] = {
            .attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
            .att_desc = {
                .uuid_length = ESP_UUID_LEN_16,
                .uuid_p = (uint8_t*)&character_declaration_uuid,
                .perm = ESP_GATT_PERM_READ,
                .max_length = sizeof(uint8_t),
                .length = sizeof(uint8_t),
                .value = (uint8_t*)&char_prop_notify
            }
        },
        // Characteristic Value
        [2] = {
            .attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
            .att_desc = {
                .uuid_length = ESP_UUID_LEN_16,
                .uuid_p = (uint8_t*)&character_notification_uuid,
                .perm = ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
                .max_length = MAX_NOTIFICATION_LENGTH,
                .length = 0
            }
        }
    };

    // Register service with correct parameters
    ret = esp_ble_gatts_create_attr_tab(gatt_db, ESP_GATT_IF_NONE, 3, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create attribute table failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register the GATT application
    ret = esp_ble_gatts_app_register(0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Add service failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t send_notification(const char* message) {
    // Check connection status using Bluetooth manager
    if (!bluetooth_manager_is_connected()) {
        ESP_LOGW(TAG, "No active connection to send notification");
        return ESP_FAIL;
    }

    // Use local buffer to reduce global memory usage
    uint8_t notify_data[MAX_NOTIFICATION_LENGTH];
    size_t len = strlen(message);
    
    // Ensure message doesn't exceed max length
    len = (len > MAX_NOTIFICATION_LENGTH) ? MAX_NOTIFICATION_LENGTH : len;
    strncpy((char*)notify_data, message, len);

    // Send notification
    esp_ble_gatts_send_indicate(
        s_notification_ctx.gatts_interface,
        s_notification_ctx.conn_id,
        s_notification_ctx.notification_handle,
        len,
        notify_data,
        false
    );

    ESP_LOGI(TAG, "Notification sent: %s", message);
    return ESP_OK;
}

uint16_t get_notification_handle(void) {
    return s_notification_ctx.notification_handle;
}