#ifndef MPU9250_H
#define MPU9250_H

#include "esp_err.h"

// MPU9250 I2C address
#define MPU9250_I2C_ADDRESS     0x68

// MPU9250 registers
#define MPU9250_REG_WHO_AM_I    0x75
#define MPU9250_REG_ACCEL_XOUT_H 0x3B
#define MPU9250_REG_GYRO_XOUT_H  0x43

typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
} mpu9250_data_t;

// Initialize MPU9250
esp_err_t mpu9250_init(int sda_pin, int scl_pin);

// Read sensor data
esp_err_t mpu9250_read_data(mpu9250_data_t *data);

#endif // MPU9250_H