#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include "max30100.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAX30100_HEART_RATE_MONITOR";
#define I2C_MASTER_NUM I2C_NUM_0

static float g_heart_rate = 0.0f;
static float g_spo2 = 0.0f;
static SemaphoreHandle_t heart_rate_mutex = NULL;

// Utility function to safely get heart rate data
bool get_heart_rate_data(float *heart_rate, float *spo2) {
    if (heart_rate_mutex == NULL) {
        return false;
    }

    if (xSemaphoreTake(heart_rate_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        *heart_rate = g_heart_rate;
        *spo2 = g_spo2;
        xSemaphoreGive(heart_rate_mutex);
        return true;
    }
    
    return false;
}