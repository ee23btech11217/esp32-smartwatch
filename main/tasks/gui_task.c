#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lvgl_helpers.h"

#define TAG "SIMPLE_TEST"
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320
#define DISP_BUF_SIZE (DISPLAY_WIDTH * 10)

// Display buffer
static lv_disp_draw_buf_t disp_buf;
static lv_color_t *buf1;

// Simple GUI task
static void simple_gui_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Starting Simple GUI Task");

    // Allocate display buffer
    buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    if (!buf1)
    {
        ESP_LOGE(TAG, "Failed to allocate display buffer");
        vTaskDelete(NULL);
        return;
    }

    // Initialize LVGL
    lv_init();

    // Initialize display and touch drivers
    lvgl_driver_init();

    // Initialize display buffer
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, DISP_BUF_SIZE);

    // Register the display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush; // Use helper driver
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = DISPLAY_WIDTH;
    disp_drv.ver_res = DISPLAY_HEIGHT;
    lv_disp_drv_register(&disp_drv);

    // Simple label creation
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello, ESP32!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Main task loop
    while (1)
    {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10)); // Add delay to avoid watchdog trigger
    }
}

void gui_task_init(void)
{
    xTaskCreatePinnedToCore(simple_gui_task, "simple_gui", 4096 * 2, NULL, 5, NULL, 1);
}
