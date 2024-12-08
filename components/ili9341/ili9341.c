#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "ili9341.h"
#include "lvgl.h"

#define TAG "ILI9341"

#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320

static spi_device_handle_t spi;
static int16_t _width = ILI9341_WIDTH;
static int16_t _height = ILI9341_HEIGHT;

// Transaction queue depth
#define PARALLEL_LINES 16

void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    t.user = (void *)0;
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0)
        return;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = data;
    t.user = (void *)1;
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

void IRAM_ATTR lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

esp_err_t ili9341_init(void)
{
    ESP_LOGI(TAG, "Initializing ILI9341");

    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PARALLEL_LINES * 320 * 2 + 8};

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000, // Increased clock speed
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = lcd_spi_pre_transfer_callback,
        .flags = SPI_DEVICE_NO_DUMMY};

    // Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    // Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Initialize SPI bus
    ret = spi_bus_initialize(LCD_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    // Initialize the LCD
    lcd_cmd(spi, 0xCF);
    uint8_t data1[] = {0x00, 0xC1, 0X30};
    lcd_data(spi, data1, 3);

    lcd_cmd(spi, 0xED);
    uint8_t data2[] = {0x64, 0x03, 0X12, 0X81};
    lcd_data(spi, data2, 4);

    lcd_cmd(spi, 0xE8);
    uint8_t data3[] = {0x85, 0x00, 0x78};
    lcd_data(spi, data3, 3);

    lcd_cmd(spi, 0xCB);
    uint8_t data4[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
    lcd_data(spi, data4, 5);

    lcd_cmd(spi, 0xF7);
    uint8_t data5[] = {0x20};
    lcd_data(spi, data5, 1);

    lcd_cmd(spi, 0xEA);
    uint8_t data6[] = {0x00, 0x00};
    lcd_data(spi, data6, 2);

    // Power Control
    lcd_cmd(spi, 0xC0);
    uint8_t data7[] = {0x23};
    lcd_data(spi, data7, 1);

    lcd_cmd(spi, 0xC1);
    uint8_t data8[] = {0x10};
    lcd_data(spi, data8, 1);

    // VCOM Control
    lcd_cmd(spi, 0xC5);
    uint8_t data9[] = {0x3e, 0x28};
    lcd_data(spi, data9, 2);

    lcd_cmd(spi, 0xC7);
    uint8_t data10[] = {0x86};
    lcd_data(spi, data10, 1);

    lcd_cmd(spi, 0x36);
    uint8_t data11[] = {0x48};
    lcd_data(spi, data11, 1);

    lcd_cmd(spi, 0x3A);
    uint8_t data12[] = {0x55};
    lcd_data(spi, data12, 1);

    lcd_cmd(spi, 0xB1);
    uint8_t data13[] = {0x00, 0x18};
    lcd_data(spi, data13, 2);

    lcd_cmd(spi, 0xB6);
    uint8_t data14[] = {0x08, 0x82, 0x27};
    lcd_data(spi, data14, 3);

    lcd_cmd(spi, 0xF2);
    uint8_t data15[] = {0x00};
    lcd_data(spi, data15, 1);

    lcd_cmd(spi, 0x26);
    uint8_t data16[] = {0x01};
    lcd_data(spi, data16, 1);

    lcd_cmd(spi, 0xE0);
    uint8_t data17[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00};
    lcd_data(spi, data17, 15);

    lcd_cmd(spi, 0xE1);
    uint8_t data18[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F};
    lcd_data(spi, data18, 15);

    lcd_cmd(spi, 0x11); // Exit sleep
    vTaskDelay(120 / portTICK_PERIOD_MS);

    lcd_cmd(spi, 0x29); // Display on

    // Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 1);

    ESP_LOGI(TAG, "ILI9341 initialized successfully");
    
    // Run display test
    ili9341_display_test();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    return ESP_OK;
}

void ili9341_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

    // Column addresses
    lcd_cmd(spi, 0x2A);
    uint8_t col_data[] = {(area->x1 >> 8) & 0xFF, area->x1 & 0xFF, (area->x2 >> 8) & 0xFF, area->x2 & 0xFF};
    lcd_data(spi, col_data, 4);

    // Page addresses
    lcd_cmd(spi, 0x2B);
    uint8_t row_data[] = {(area->y1 >> 8) & 0xFF, area->y1 & 0xFF, (area->y2 >> 8) & 0xFF, area->y2 & 0xFF};
    lcd_data(spi, row_data, 4);

    // Memory write
    lcd_cmd(spi, 0x2C);

    // Send data
    lcd_data(spi, (uint8_t *)color_p, size * 2);

    lv_disp_flush_ready(disp_drv);
}

void ili9341_set_rotation(lcd_rotation_t rotation)
{
    uint8_t data;
    switch (rotation)
    {
    case LCD_ROTATE_0:
        data = (1 << 3);
        _width = ILI9341_WIDTH;
        _height = ILI9341_HEIGHT;
        break;
    case LCD_ROTATE_90:
        data = (1 << 3) | (1 << 5) | (1 << 6);
        _width = ILI9341_HEIGHT;
        _height = ILI9341_WIDTH;
        break;
    case LCD_ROTATE_180:
        data = (1 << 3) | (1 << 7);
        _width = ILI9341_WIDTH;
        _height = ILI9341_HEIGHT;
        break;
    case LCD_ROTATE_270:
        data = (1 << 3) | (1 << 5) | (1 << 7);
        _width = ILI9341_HEIGHT;
        _height = ILI9341_WIDTH;
        break;
    }
    lcd_cmd(spi, 0x36); // Memory Access Control
    lcd_data(spi, &data, 1);
}

void ili9341_display_test(void)
{
    // Set column address
    lcd_cmd(spi, 0x2A);
    uint8_t col_data[] = {0x00, 0x00, (_width - 1) >> 8, (_width - 1) & 0xFF};
    lcd_data(spi, col_data, 4);

    // Set row address
    lcd_cmd(spi, 0x2B);
    uint8_t row_data[] = {0x00, 0x00, (_height - 1) >> 8, (_height - 1) & 0xFF};
    lcd_data(spi, row_data, 4);

    // Memory write
    lcd_cmd(spi, 0x2C);

    // Create color bars
    uint16_t colors[] = {
        0xF800, // Red
        0x07E0, // Green
        0x001F, // Blue
        0xFFE0, // Yellow
        0xF81F, // Magenta
        0x07FF, // Cyan
        0x0000, // Black
        0xFFFF  // White
    };

    int total_pixels = _width * _height;
    int pixels_per_color = total_pixels / 8;

    for (int i = 0; i < total_pixels; i++)
    {
        int color_index = i / pixels_per_color;
        if (color_index >= 8)
            color_index = 7; // Ensure we don't go out of bounds
        uint8_t color_bytes[] = {colors[color_index] >> 8, colors[color_index] & 0xFF};
        lcd_data(spi, color_bytes, 2);
    }
}
