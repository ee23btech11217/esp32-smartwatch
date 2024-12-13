#ifndef NOTIFY_H
#define NOTIFY_H

#include "esp_err.h"
#include "esp_gatts_api.h"

// Maximum notification length
#define MAX_NOTIFICATION_LENGTH 50

// Initialize BLE GATT service for notifications
esp_err_t notification_init(void);

// Send a notification to connected device
esp_err_t send_notification(const char* message);

// Get current notification handle
uint16_t get_notification_handle(void);

#endif // NOTIFY_H