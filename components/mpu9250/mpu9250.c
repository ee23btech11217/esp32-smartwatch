#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "mpu9250.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000  
#define I2C_TIMEOUT_MS 1000

static const char *TAG = "MPU9250";

static esp_err_t i2c_master_init(int sda_pin, int scl_pin)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }

    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

static esp_err_t mpu9250_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU9250_I2C_ADDRESS, &reg_addr, 1, data, len, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t mpu9250_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_I2C_ADDRESS, write_buf, sizeof(write_buf), I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t mpu9250_init(int sda_pin, int scl_pin)
{
    esp_err_t err = i2c_master_init(sda_pin, scl_pin);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master");
        return err;
    }

    // Check device ID
    uint8_t who_am_i;
    err = mpu9250_register_read(MPU9250_REG_WHO_AM_I, &who_am_i, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I register");
        return err;
    }

    if (who_am_i != 0x71) {
        ESP_LOGE(TAG, "Unknown device ID: 0x%02x", who_am_i);
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "MPU9250 found");
    
    // Wake up the device
    err = mpu9250_register_write_byte(0x6B, 0x00);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up the device");
        return err;
    }

    return ESP_OK;
}

esp_err_t mpu9250_read_data(mpu9250_data_t *data)
{
    uint8_t buffer[14];
    esp_err_t err = mpu9250_register_read(MPU9250_REG_ACCEL_XOUT_H, buffer, 14);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read sensor data");
        return err;
    }

    // Convert the data
    int16_t accel_x = (buffer[0] << 8) | buffer[1];
    int16_t accel_y = (buffer[2] << 8) | buffer[3];
    int16_t accel_z = (buffer[4] << 8) | buffer[5];
    int16_t gyro_x = (buffer[8] << 8) | buffer[9];
    int16_t gyro_y = (buffer[10] << 8) | buffer[11];
    int16_t gyro_z = (buffer[12] << 8) | buffer[13];

    // Convert to float (adjust scale based on your settings)
    const float accel_scale = 1.0f / 16384.0f; // For ±2g
    const float gyro_scale = 1.0f / 131.0f;    // For ±250 degrees/sec

    data->accel_x = accel_x * accel_scale;
    data->accel_y = accel_y * accel_scale;
    data->accel_z = accel_z * accel_scale;
    data->gyro_x = gyro_x * gyro_scale;
    data->gyro_y = gyro_y * gyro_scale;
    data->gyro_z = gyro_z * gyro_scale;

    return ESP_OK;
}