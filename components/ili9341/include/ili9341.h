#pragma once

#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

// Pin definitions
#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

#define PIN_NUM_DC   21
#define PIN_NUM_RST  22
#define PIN_NUM_BCKL 4

// Display rotation
typedef enum {
    LCD_ROTATE_0 = 0,
    LCD_ROTATE_90 = 1,
    LCD_ROTATE_180 = 2,
    LCD_ROTATE_270 = 3
} lcd_rotation_t;

// Function declarations
esp_err_t ili9341_init(void);
void ili9341_set_rotation(lcd_rotation_t rotation);
void ili9341_display_test(void);
void ili9341_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);