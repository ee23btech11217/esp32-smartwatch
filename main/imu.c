#include "mpu9250.h"
#include "esp_log.h"
#include "esp_err.h"

static const char* TAG = "MPU9250";
#define I2C_MASTER_NUM I2C_NUM_0

void mpu9250_init(void) {
    // Configuration structure for MPU9250
    mpu9250_config_t config = {
        .accel_enabled = true,
        .gyro_enabled = true,
        .temp_enabled = true,
        .accel_filter_level = 4,
        .gyro_temp_filter_level = 4,
    };

    // MPU9250 instance
    mpu9250_t mpu;

    // MPU9250 I2C address
    int address = 0x68;  // Ensure this matches the hardware configuration (AD0 pin)

    // Initialize MPU9250
    esp_err_t ret = mpu9250_begin(&mpu, config, address, I2C_MASTER_NUM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MPU9250: %s", esp_err_to_name(ret));
        return;
    }

    // Update sensor data and check for successful reading
    ret = mpu9250_update(&mpu, I2C_MASTER_NUM, address);
    if (ret == 1) {
        ESP_LOGI(TAG, "Accelerometer - X: %d, Y: %d, Z: %d", 
                 mpu.accel.x, mpu.accel.y, mpu.accel.z);

        ESP_LOGI(TAG, "Gyroscope - X: %d, Y: %d, Z: %d", 
                 mpu.gyro.x, mpu.gyro.y, mpu.gyro.z);

        ESP_LOGI(TAG, "Temperature: %.2f°C", mpu.temp);
    } else {
        ESP_LOGW(TAG, "No new data available from MPU9250.");
    }
}
