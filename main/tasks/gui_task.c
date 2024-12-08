#include "gui_task.h"
#include <time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "ili9341.h"

#define TAG "GUI_TASK"

#ifndef ILI9341_WIDTH
#define ILI9341_WIDTH  240
#endif
#ifndef ILI9341_HEIGHT
#define ILI9341_HEIGHT 320
#endif

#define DISP_BUF_SIZE (ILI9341_WIDTH * 16)
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf1[DISP_BUF_SIZE];
static lv_color_t buf2[DISP_BUF_SIZE];

// Global variables for GUI elements
static lv_obj_t *time_label;
static lv_style_t style_text;

static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(1);
}

static void gui_task(void *pvParameter)
{
    (void)pvParameter;

    ESP_LOGI(TAG, "Starting GUI Task");

    lv_init();
    
    // Initialize ILI9341
    ESP_ERROR_CHECK(ili9341_init());
    ili9341_set_rotation(LCD_ROTATE_180);
    ESP_LOGI(TAG, "ILI9341 initialized");

    // Initialize display buffer
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = ili9341_flush;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = ILI9341_WIDTH;
    disp_drv.ver_res = ILI9341_HEIGHT;

    lv_disp_drv_register(&disp_drv);
    ESP_LOGI(TAG, "Display driver registered");

    // Create and start the periodic timer for LVGL
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000));

    // Create a simple watch face
    lv_style_init(&style_text);
    lv_style_set_text_color(&style_text, lv_color_make(255, 255, 255));
    lv_style_set_text_font(&style_text, &lv_font_montserrat_14);

    lv_obj_t *scr = lv_scr_act();

    // Set screen background color
    lv_obj_set_style_bg_color(scr, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Create time label
    time_label = lv_label_create(scr);
    lv_obj_add_style(time_label, &style_text, 0);
    lv_label_set_text(time_label, "00:00:00");
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);

    // Create orientation markers
    lv_obj_t *tl_label = lv_label_create(scr);
    lv_obj_add_style(tl_label, &style_text, 0);
    lv_label_set_text(tl_label, "TL");
    lv_obj_align(tl_label, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t *br_label = lv_label_create(scr);
    lv_obj_add_style(br_label, &style_text, 0);
    lv_label_set_text(br_label, "BR");
    lv_obj_align(br_label, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

    ESP_LOGI(TAG, "GUI elements created");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        lv_timer_handler();

        // Get the current time
        time_t now;
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);
        char timeString[9];
        snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", 
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        // Update the time label
        lv_label_set_text(time_label, timeString);
    }
}

void gui_task_init(void)
{
    xTaskCreatePinnedToCore(gui_task, "gui", 4096 * 2, NULL, 5, NULL, 1);
}