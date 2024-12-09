#include <stdio.h>
#include <time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "wifi_manager.h"
#include "tasks/gui_task.h"

#define TAG "SMARTWATCH"
#define I2C_SDA_PIN 32
#define I2C_SCL_PIN 33

void app_main(void)
{
    // Initialize WiFi
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(wifi_manager_connect("Jio 1", "raja1234"));

    // Wait for connection with timeout
    int retry_count = 0;
    while (wifi_manager_get_state() != WIFI_CONNECTED && retry_count < 10)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry_count++;
    }

    if (wifi_manager_get_state() != WIFI_CONNECTED)
    {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
    }
    else
    {
        ESP_LOGI(TAG, "WiFi connected");
    }

    // Initialize SNTP
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // Set timezone
    setenv("TZ", "IST-5:30", 1);
    tzset();

    // Start GUI task
    ESP_LOGI(TAG, "Starting GUI application");
    gui_task_init();
}