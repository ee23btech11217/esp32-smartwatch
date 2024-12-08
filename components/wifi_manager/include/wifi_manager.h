#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi connection states
 */
typedef enum {
    WIFI_DISCONNECTED = 0,
    WIFI_CONNECTING,
    WIFI_CONNECTED
} wifi_connection_state_t;

/**
 * @brief Initialize WiFi as station
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Connect to WiFi network
 *
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password);

/**
 * @brief Get current WiFi connection state
 *
 * @return wifi_connection_state_t Current connection state
 */
wifi_connection_state_t wifi_manager_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_MANAGER_H */