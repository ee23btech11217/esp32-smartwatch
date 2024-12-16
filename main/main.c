#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "wifiManager.h"
#include "lvglPort.h"
#include "max30100.h"

static const char *TAG = "main";

#define EXAMPLE_PIN_NUM_SCLK 18
#define EXAMPLE_PIN_NUM_MOSI 23
#define EXAMPLE_PIN_NUM_MISO 19
#define HOST SPI2_HOST

#define I2C_MASTER_SCL_IO 25
#define I2C_MASTER_SDA_IO 26
#define I2C_MASTER_NUM I2C_NUM_0

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_DELAY_MS 50

static SemaphoreHandle_t lvgl_mux = NULL;
static float g_heart_rate = 0.0f;
static float g_spo2 = 0.0f;
static SemaphoreHandle_t heart_rate_mutex = NULL;

extern void create_watch_face(lv_disp_t *disp, float heart_rate, float spo2);
extern void configure_system_time();
extern void wifi_init_sta();
extern void mpu9250_init();

bool lvgl_lock(int timeout_ms) {
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

void lvgl_unlock(void) {
    xSemaphoreGiveRecursive(lvgl_mux);
}

void heart_rate_monitor_task(void *pvParameters) {
    heart_rate_mutex = xSemaphoreCreateMutex();
    max30100_config_t max30100_config;
    max30100_data_t max30100_data;

    max30100_init(
        &max30100_config,
        I2C_MASTER_NUM,
        MAX30100_MODE_SPO2_HR,
        MAX30100_SAMPLING_RATE_100HZ,
        MAX30100_PULSE_WIDTH_1600US_ADC_16,
        MAX30100_LED_CURRENT_50MA,
        MAX30100_LED_CURRENT_27_1MA,
        15, 10, true, false
    );

    while (1) {
        if (max30100_update(&max30100_config, &max30100_data) == ESP_OK) {
            if (xSemaphoreTake(heart_rate_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (max30100_data.pulse_detected) {
                    g_heart_rate = max30100_data.heart_bpm;
                    g_spo2 = max30100_data.spO2;
                }
                xSemaphoreGive(heart_rate_mutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void lvgl_port_task(void *arg) {
    lv_disp_t *disp = (lv_disp_t *)arg;
    float current_heart_rate = 0.0f;
    float current_spo2 = 0.0f;

    while (1) {
        if (xSemaphoreTake(heart_rate_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            current_heart_rate = g_heart_rate;
            current_spo2 = g_spo2;
            xSemaphoreGive(heart_rate_mutex);
        }

        if (lvgl_lock(-1)) {
            create_watch_face(disp, current_heart_rate, current_spo2);
            lv_timer_handler();
            lvgl_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(EXAMPLE_LVGL_TASK_DELAY_MS));
    }
}

void increase_lvgl_tick(void *arg) {
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void app_main(void) {
    lv_init();

    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(wifi_manager_connect("Jio 1", "raja1234"));

    configure_system_time();

    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0));

    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 38400,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &buscfg, SPI_DMA_CH_AUTO));

    lv_disp_t *disp = lvgl_port_init();

    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    mpu9250_init();

    xTaskCreatePinnedToCore(heart_rate_monitor_task, "Heart Rate Monitor", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(lvgl_port_task, "LVGL", 4096, disp, 3, NULL, 1);
}