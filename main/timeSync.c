#include <stdio.h>
#include <time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

static const char *TAG = "CLOCK_SYNC";

void check_wifi_connection(void) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        ESP_LOGI(TAG, "Connected to Wi-Fi: %s", ap_info.ssid);
    } else {
        ESP_LOGE(TAG, "Wi-Fi is not connected");
    }
}

void configure_system_time(void) {

    // Check Wi-Fi connection
    check_wifi_connection();

    // Initialize SNTP
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // Set timezone for Indian Standard Time
    setenv("TZ", "IST-5:30", 1);
    tzset();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // Get current time
    time(&now);
    localtime_r(&now, &timeinfo);

    // Check if time is valid (after 2000)
    if (timeinfo.tm_year < 100) {
        ESP_LOGE(TAG, "Failed to set system time. Year is %d", timeinfo.tm_year + 1900);
        return;
    }

    // Log current time
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Current time: %s", strftime_buf);
}
