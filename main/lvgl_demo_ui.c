#include "lvgl.h"
#include "time.h"

static lv_obj_t *digital_label = NULL;
static lv_obj_t *analog_clock = NULL;
static lv_meter_indicator_t *sec_indic = NULL;
static lv_meter_indicator_t *min_indic = NULL;
static lv_meter_indicator_t *hour_indic = NULL;
static lv_obj_t *status_button = NULL;
static bool button_pressed = false;

static void button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED)
    {
        button_pressed = !button_pressed;
        
        if (button_pressed)
        {
            // Change button to green when pressed
            lv_obj_set_style_bg_color(status_button, lv_palette_main(LV_PALETTE_GREEN), 0);
        }
        else
        {
            // Return to default color when released
            lv_obj_set_style_bg_color(status_button, lv_palette_main(LV_PALETTE_GREY), 0);
        }
    }
}

static void update_clock(lv_timer_t *timer)
{
    if (!digital_label || !analog_clock ||
        !sec_indic || !min_indic || !hour_indic)
    {
        return;
    }

    time_t now;
    struct tm timeinfo;

    // Get current time safely
    time(&now);
    if (localtime_r(&now, &timeinfo) == NULL)
    {
        return;
    }

    // Update digital clock
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
    lv_label_set_text(digital_label, time_str);

    // Calculate precise values for analog clock
    int32_t sec_val = timeinfo.tm_sec * 100 / 60;
    int32_t min_val = timeinfo.tm_min * 100 / 60;
    int32_t hour_val = ((timeinfo.tm_hour % 12) * 100 / 12) + (min_val / 60);

    // Safely update analog clock indicators
    lv_meter_set_indicator_value(analog_clock, sec_indic, sec_val);
    lv_meter_set_indicator_value(analog_clock, min_indic, min_val);
    lv_meter_set_indicator_value(analog_clock, hour_indic, hour_val);
}

void example_lvgl_demo_ui(lv_disp_t *disp)
{
    // Ensure clean slate
    digital_label = NULL;
    analog_clock = NULL;
    sec_indic = NULL;
    min_indic = NULL;
    hour_indic = NULL;
    status_button = NULL;

    lv_obj_t *scr = lv_disp_get_scr_act(disp);

    // Digital Clock Label
    digital_label = lv_label_create(scr);
    lv_obj_set_style_text_font(digital_label, &lv_font_montserrat_32, 0);
    lv_obj_align(digital_label, LV_ALIGN_TOP_MID, 0, 20);

    // Analog Clock (Meter)
    analog_clock = lv_meter_create(scr);
    lv_obj_set_size(analog_clock, 250, 250);
    lv_obj_center(analog_clock);

    // Create scale
    lv_meter_scale_t *scale = lv_meter_add_scale(analog_clock);
    
    // Add major hour markers (12 markers)
    lv_meter_set_scale_ticks(analog_clock, scale, 12, 3, 15, lv_color_black());
    
    // Add minor minute markers
    lv_meter_set_scale_ticks(analog_clock, scale, 60, 1, 5, lv_palette_main(LV_PALETTE_GREY));
    
    // Set scale range to create a clock-like circular scale
    lv_meter_set_scale_range(analog_clock, scale, 0, 100, 360, 270);

    // Seconds indicator (thin, long red)
    sec_indic = lv_meter_add_needle_line(analog_clock, scale, 2, 
                                         lv_palette_main(LV_PALETTE_RED), -20);

    // Minutes indicator (medium, shorter blue)
    min_indic = lv_meter_add_needle_line(analog_clock, scale, 4, 
                                         lv_palette_main(LV_PALETTE_BLUE), -15);

    // Hours indicator (thick, shortest grey)
    hour_indic = lv_meter_add_needle_line(analog_clock, scale, 6, 
                                          lv_palette_main(LV_PALETTE_GREY), -10);

    // Status Button
    status_button = lv_btn_create(scr);
    lv_obj_set_size(status_button, 100, 50);
    lv_obj_align(status_button, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(status_button, button_event_handler, LV_EVENT_ALL, NULL);

    // Button Label
    lv_obj_t *btn_label = lv_label_create(status_button);
    lv_label_set_text(btn_label, "Status");
    lv_obj_center(btn_label);

    // Create a timer to update clock every second
    lv_timer_create(update_clock, 1000, NULL);
}