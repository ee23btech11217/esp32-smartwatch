#include <stdio.h>
#include <time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "wifi_manager.h"
#include "tasks/gui_task.h"
#include "mpu9250.h"

#define TAG "SMARTWATCH"
#define I2C_SDA_PIN 32
#define I2C_SCL_PIN 33

void mpu9250_test_task(void *pvParameters)
{
    mpu9250_data_t sensor_data;

    while (1)
    {
        if (mpu9250_read_data(&sensor_data) == ESP_OK)
        {
            ESP_LOGI(TAG, "Accel: X=%.2f Y=%.2f Z=%.2f G",
                     sensor_data.accel_x, sensor_data.accel_y, sensor_data.accel_z);
            ESP_LOGI(TAG, "Gyro: X=%.2f Y=%.2f Z=%.2f deg/s",
                     sensor_data.gyro_x, sensor_data.gyro_y, sensor_data.gyro_z);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read MPU9250 data");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

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

    // Initialize MPU9250
    esp_err_t ret = mpu9250_init(I2C_SDA_PIN, I2C_SCL_PIN);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize MPU9250");
    }
    else
    {
        ESP_LOGI(TAG, "MPU9250 initialized successfully");
    }

    // xTaskCreate(mpu9250_test_task, "mpu9250_test", 2048, NULL, 5, NULL);

    // Start GUI task
    ESP_LOGI(TAG, "Starting GUI application");
    gui_task_init();
}