#include "mpu9250.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define TIMEOUT 1000 // Timeout in milliseconds
#define ERR_CHECK(x)                                                           \
  {                                                                            \
    int err = (x);                                                             \
    if (err != ESP_OK)                                                         \
      return x;                                                                \
  }

static const char *TAG = "mpu9250";

int mpu9250_begin(mpu9250_t *mpu, const mpu9250_config_t config, int address,
                  i2c_port_t i2c_port) {
  ESP_LOGD(TAG, "Initiating connection with MPU9250 at address %#02X", address);

  // Check if device exists at address
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
  i2c_master_stop(cmd);
  esp_err_t err = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(TIMEOUT));
  i2c_cmd_link_delete(cmd);
  ERR_CHECK(err);
  ESP_LOGD(TAG, "Device exists");

  // Reset MPU
  uint8_t pwr_mgmt_1[] = {
      0x6B,        // PWR_MGMT_1
      1 << 7 |     // H_RESET => Reset the MPU9250
          0 << 6 | // SLEEP
          0 << 5 | // CYCLE
          0 << 4 | // GYRO_STANDBY
          0 << 3 | // PD_PTAT
          1        // CLKSEL => Select best clock source
  };
  ERR_CHECK(i2c_master_write_to_device(i2c_port, address, pwr_mgmt_1, sizeof(pwr_mgmt_1), pdMS_TO_TICKS(TIMEOUT)));

  // Configure MPU
  uint8_t pwr_mgmt_2[] = {
      0x6C,                            // PWR_MGMT_2
      !config.accel_enabled << 5 |     // Accel X
          !config.accel_enabled << 4 | // Accel Y
          !config.accel_enabled << 3 | // Accel Z
          !config.gyro_enabled << 2 |  // Gyro X
          !config.gyro_enabled << 1 |  // Gyro Y
          !config.gyro_enabled         // Gyro Z
  };
  ERR_CHECK(i2c_master_write_to_device(i2c_port, address, pwr_mgmt_2, sizeof(pwr_mgmt_2), pdMS_TO_TICKS(TIMEOUT)));

  int dlpf_cfg = 0;
  int fchoice_b = 0b11;
  switch (config.gyro_temp_filter_level) {
  case 0:
    fchoice_b = 0b01;
    break;
  case 1:
    fchoice_b = 0b10;
    break;
  default:
    dlpf_cfg = config.gyro_temp_filter_level - 2;
  }
  uint8_t reg_config[] = {
      0x1A,        // CONFIG
      0 << 3 |     // Disable FSYNC
          dlpf_cfg // DLPF_CFG
  };
  ERR_CHECK(i2c_master_write_to_device(i2c_port, address, reg_config, sizeof(reg_config), pdMS_TO_TICKS(TIMEOUT)));

  if (config.gyro_enabled) {
    ESP_LOGD(TAG, "Configuring gyroscope settings");
    uint8_t gyro_config[] = {
        0x1B,             // GYRO_CONFIG
        0 << 4 | 0 << 3 | // GYRO_FS_SEL => Set scale to 250 degrees per second
            fchoice_b     // FCHOICE_B
    };
    ERR_CHECK(i2c_master_write_to_device(i2c_port, address, gyro_config, sizeof(gyro_config), pdMS_TO_TICKS(TIMEOUT)));
  }
  if (config.accel_enabled) {
    ESP_LOGD(TAG, "Configuring accelerometer settings");
    uint8_t accel_config[] = {
        0x1C,           // ACCEL_CONFIG
        0 << 4 | 0 << 3 // ACCEL_FS_SEL => Set scale to +-2g
    };
    ERR_CHECK(i2c_master_write_to_device(i2c_port, address, accel_config, sizeof(accel_config), pdMS_TO_TICKS(TIMEOUT)));

    int accel_fchoice_b = 1;
    int accel_dlpf_cfg = 0;
    switch (config.accel_filter_level) {
    case 0:
      accel_fchoice_b = 0;
      break;
    default:
      accel_dlpf_cfg = config.accel_filter_level - 1;
    }
    uint8_t accel_config_2[] = {
        0x1D,                  // ACCEL_CONFIG_2
        accel_fchoice_b << 3 | // FCHOICE_B
            accel_dlpf_cfg     // A_DLPFCFG
    };
    ERR_CHECK(i2c_master_write_to_device(i2c_port, address, accel_config_2, sizeof(accel_config_2), pdMS_TO_TICKS(TIMEOUT)));
  }

  uint8_t int_enable[] = {
      0x38, // INT_ENABLE
      1     // RAW_RDY_EN => Trigger when raw sensor data is ready
  };
  ERR_CHECK(i2c_master_write_to_device(i2c_port, address, int_enable, sizeof(int_enable), pdMS_TO_TICKS(TIMEOUT)));

  ESP_LOGD(TAG, "Configuration complete");
  mpu->config = config;
  return 0;
}

int mpu9250_update(mpu9250_t *mpu, i2c_port_t i2c_port, int address) {
  uint8_t int_status = 0x3A;
  uint8_t result;
  ERR_CHECK(i2c_master_write_read_device(i2c_port, address, &int_status, 1, &result, 1, pdMS_TO_TICKS(TIMEOUT)));
  ESP_LOGD(TAG, "INT_STATUS: %d", result);

  if (result & 1) {
    uint8_t read_buffer[6];
    if (mpu->config.accel_enabled) {
      const uint8_t accel_start_reg = 0x3B;
      ERR_CHECK(i2c_master_write_read_device(i2c_port, address, &accel_start_reg, 1, read_buffer, 6, pdMS_TO_TICKS(TIMEOUT)));
      mpu->accel.x = (int16_t)(read_buffer[0] << 8 | read_buffer[1]);
      mpu->accel.y = (int16_t)(read_buffer[2] << 8 | read_buffer[3]);
      mpu->accel.z = (int16_t)(read_buffer[4] << 8 | read_buffer[5]);
    }

    if (mpu->config.temp_enabled) {
      const uint8_t temp_start_reg = 0x41;
      ERR_CHECK(i2c_master_write_read_device(i2c_port, address, &temp_start_reg, 1, read_buffer, 2, pdMS_TO_TICKS(TIMEOUT)));
      mpu->temp = (int16_t)(read_buffer[0] << 8 | read_buffer[1]) / 333.87 + 21;
    }

    if (mpu->config.gyro_enabled) {
      const uint8_t gyro_start_reg = 0x43;
      ERR_CHECK(i2c_master_write_read_device(i2c_port, address, &gyro_start_reg, 1, read_buffer, 6, pdMS_TO_TICKS(TIMEOUT)));
      mpu->gyro.x = (int16_t)(read_buffer[0] << 8 | read_buffer[1]);
      mpu->gyro.y = (int16_t)(read_buffer[2] << 8 | read_buffer[3]);
      mpu->gyro.z = (int16_t)(read_buffer[4] << 8 | read_buffer[5]);
    }

    return 1;
  }

  return 0;
}
