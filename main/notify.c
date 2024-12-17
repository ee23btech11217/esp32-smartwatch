#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "nvs_flash.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "BLE_CUSTOM_NOTIFY";
uint8_t ble_addr_type;
void ble_app_advertise(void);

// Custom service and characteristic UUIDs
#define CUSTOM_SERVICE_UUID 0x1802
#define CUSTOM_CHAR_UUID 0x2a02

// Custom notification data
#define CUSTOM_NOTIFY_MESSAGE "Hello from ESP32 Custom Service!"

// Custom write handler
static int custom_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    ESP_LOGI(TAG, "Custom characteristic accessed.");

    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        ESP_LOGI(TAG, "Received WRITE: %.*s", ctxt->om->om_len, ctxt->om->om_data);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
    {
        const char *msg = "SPP_CHR";
        os_mbuf_append(ctxt->om, msg, strlen(msg));
        ESP_LOGI(TAG, "Responded to READ with: %s", msg);
    }

    return 0;
}

// Define characteristics and service
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(CUSTOM_SERVICE_UUID),
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(CUSTOM_CHAR_UUID),
          .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
          .access_cb = custom_handler},
         {0}}},
    {0}};

// BLE GAP event handler
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise(); // Restart advertising if connection failed
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECTED");
        ble_app_advertise(); // Restart advertising after disconnect
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "BLE GAP EVENT: Advertisement Complete");
        ble_app_advertise(); // Restart advertising
        break;

    case BLE_GAP_EVENT_CONN_UPDATE:
        ESP_LOGI(TAG, "BLE GAP EVENT: Connection parameters updated.");
        break;

    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        ESP_LOGI(TAG, "BLE GAP EVENT: Connection update request received.");
        break;

    case BLE_GAP_EVENT_ENC_CHANGE:
        ESP_LOGI(TAG, "BLE GAP EVENT: Encryption status changed.");
        break;

    case BLE_GAP_EVENT_NOTIFY_TX:
        ESP_LOGI(TAG, "BLE GAP EVENT: Notification sent.");
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "BLE GAP EVENT: Client %s notifications for handle %d.",
                 event->subscribe.cur_notify ? "subscribed to" : "unsubscribed from",
                 event->subscribe.attr_handle);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "BLE GAP EVENT: MTU size updated to %d", event->mtu.value);
        break;

    default:
        ESP_LOGW(TAG, "Unhandled GAP event: %d", event->type);
        break;
    }
    return 0;
}

// Start BLE advertising
void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    const char *device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params = {0};
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
    ESP_LOGI(TAG, "BLE advertising started");
}

// BLE stack sync callback
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
    ESP_LOGI(TAG, "BLE stack synced, advertising started");
}

// NimBLE Host Task
void host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
}

void start_bluetooth_notify_task()
{
    ESP_LOGI(TAG, "Starting BLE Custom Notify Task");

    nimble_port_init();

    ble_svc_gap_device_name_set("Smart-watch");
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}
