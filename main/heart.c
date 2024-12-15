#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include "max30100.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAX30100_HEART_RATE_MONITOR";
#define I2C_MASTER_NUM I2C_NUM_0

void heart_rate_monitor_task(void *pvParameters)
{

    max30100_config_t max30100_config;
    max30100_data_t max30100_data;

    // Initialize MAX30100
    ESP_ERROR_CHECK(max30100_init(
        &max30100_config,                   // Configuration structure
        I2C_MASTER_NUM,                                  // I2C port number
        MAX30100_MODE_SPO2_HR,              // Mode: SpO2 and Heart Rate
        MAX30100_SAMPLING_RATE_100HZ,       // Sampling rate
        MAX30100_PULSE_WIDTH_1600US_ADC_16, // Pulse width
        MAX30100_LED_CURRENT_50MA,          // IR LED current
        MAX30100_LED_CURRENT_27_1MA,        // Red LED current
        15,                                 // Mean filter size
        10,                                 // BPM sample size
        true,                               // High resolution mode
        false                               // Debug mode off
        ));

    ESP_LOGI(TAG, "Heart-rate initialised");

    while (1)
    {

        // Update sensor readings
        esp_err_t result = max30100_update(&max30100_config, &max30100_data);

        if (result == ESP_OK)
        {
            if (max30100_data.pulse_detected)
            {
                ESP_LOGI(TAG, "Pulse Detected!");
                ESP_LOGI(TAG, "Heart Rate: %.2f BPM", max30100_data.heart_bpm);
                ESP_LOGI(TAG, "SpO2: %.2f%%", max30100_data.spO2);
            }
            ESP_LOGI(TAG, "Pulse not detected");
        } else {
            ESP_LOGE(TAG, "Failed to update MAX30100 sensor: %s", esp_err_to_name(result));
        }

        // Delay to prevent too frequent readings (adjust as needed)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
