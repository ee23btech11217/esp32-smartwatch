/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

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
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "wifiManager.h"
#include "lvglPort.h"
#include "esp_task_wdt.h"
#include "driver/i2c.h"

static const char *TAG = "main";

#define EXAMPLE_PIN_NUM_SCLK 18
#define EXAMPLE_PIN_NUM_MOSI 23
#define EXAMPLE_PIN_NUM_MISO 19
#define HOST SPI2_HOST

#define I2C_MASTER_SCL_IO 25
#define I2C_MASTER_SDA_IO 26
#define I2C_MASTER_NUM I2C_NUM_0

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 10
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_STACK_SIZE (4 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY 3

static SemaphoreHandle_t lvgl_mux = NULL;

////////////////////////////////////////////////////////
extern void create_watch_face(lv_disp_t *disp);
extern void configure_system_time();
extern void wifi_init_sta();
extern void mpu9250_init();
extern void start_heart_rate_monitoring();
////////////////////////////////////////////////////////

static void increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

bool lvgl_lock(int timeout_ms)
{
    // Convert timeout in milliseconds to FreeRTOS ticks
    // If `timeout_ms` is set to -1, the program will block until the condition is met
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

void lvgl_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_mux);
}

static void lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    while (1)
    {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        if (lvgl_lock(-1))
        {
            task_delay_ms = lv_timer_handler();
            // Release the mutex
            lvgl_unlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS)
        {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        }
        else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS)
        {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

// Add these to your initialization code
void check_reset_reason()
{
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
        case ESP_RST_POWERON:
            ESP_LOGI(TAG, "Reset due to power-on event");
            break;
        case ESP_RST_EXT:
            ESP_LOGI(TAG, "Reset due to external pin");
            break;
        case ESP_RST_SW:
            ESP_LOGI(TAG, "Software reset");
            break;
        case ESP_RST_PANIC:
            ESP_LOGI(TAG, "Reset due to exception/panic");
            break;
        case ESP_RST_INT_WDT:
            ESP_LOGI(TAG, "Reset due to interrupt watchdog");
            break;
        case ESP_RST_TASK_WDT:
            ESP_LOGI(TAG, "Reset due to task watchdog");
            break;
        case ESP_RST_WDT:
            ESP_LOGI(TAG, "Reset due to other watchdog");
            break;
        case ESP_RST_DEEPSLEEP:
            ESP_LOGI(TAG, "Reset after deep sleep");
            break;
        default:
            ESP_LOGI(TAG, "Unknown reset reason");
    }
}

void ui_task(void *pvParameters)
{
    lv_disp_t *disp = (lv_disp_t *)pvParameters;

    // Initial setup
    if (lvgl_lock(pdMS_TO_TICKS(1000)))
    {
        create_watch_face(disp);
        lvgl_unlock();
    }

    // Continuous UI event handling
    while (1) // Infinite loop
    {
        // Lock LVGL mutex and process events
        if (lvgl_lock(pdMS_TO_TICKS(100)))
        {
            lv_timer_handler(); // Process LVGL timers
            lvgl_unlock();
        }

        // Delay to prevent starvation of lower-priority tasks
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    check_reset_reason();

}

void app_main(void)
{
    // Initialize LVGL before any other LVGL-related operations
    lv_init();

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

    // Configure system time
    configure_system_time();

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));

    ESP_LOGI(TAG, "Initialize SPI bus");
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
    if (disp == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    if (lvgl_mux == NULL)
    {
        ESP_LOGE(TAG, "Failed to create LVGL mutex");
        return;
    }

    mpu9250_init();
    start_heart_rate_monitoring();

    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);

    // Create UI task with display as parameter
    xTaskCreatePinnedToCore(
        ui_task,
        "UI_Task",
        8192, // Increased stack size
        disp,
        EXAMPLE_LVGL_TASK_PRIORITY + 1,
        NULL,
        1);
}