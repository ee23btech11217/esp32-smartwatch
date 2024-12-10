#include "lvgl.h"
#include "time.h"

static lv_obj_t *digital_label;
static lv_obj_t *analog_clock;
static lv_disp_rot_t rotation = LV_DISP_ROT_NONE;

static void update_clock(lv_timer_t *timer) {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    // Update digital clock
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
    lv_label_set_text(digital_label, time_str);

    // Update analog clock
    if (analog_clock) {
        // Set hours, minutes, seconds indicators
        lv_meter_set_indicator_value(analog_clock, timer->user_data, timeinfo.tm_sec * 100 / 60);
        lv_meter_set_indicator_value(analog_clock, (void*)((uintptr_t)timer->user_data + 1), 
                                     timeinfo.tm_min * 100 / 60);
        lv_meter_set_indicator_value(analog_clock, (void*)((uintptr_t)timer->user_data + 2), 
                                     (timeinfo.tm_hour % 12) * 100 / 12);
    }
}

static void btn_cb(lv_event_t *e) {
    lv_disp_t *disp = lv_event_get_user_data(e);
    rotation++;
    if (rotation > LV_DISP_ROT_270) {
        rotation = LV_DISP_ROT_NONE;
    }
    lv_disp_set_rotation(disp, rotation);
}

void example_lvgl_demo_ui(lv_disp_t *disp) {
    lv_obj_t *scr = lv_disp_get_scr_act(disp);

    // Digital Clock Label
    digital_label = lv_label_create(scr);
    lv_obj_set_style_text_font(digital_label, &lv_font_montserrat_32, 0);
    lv_obj_align(digital_label, LV_ALIGN_TOP_MID, 0, 20);

    // Analog Clock (Meter)
    analog_clock = lv_meter_create(scr);
    lv_obj_set_size(analog_clock, 200, 200);
    lv_obj_center(analog_clock);

    // Create scale
    lv_meter_scale_t *scale = lv_meter_add_scale(analog_clock);
    lv_meter_set_scale_ticks(analog_clock, scale, 12, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(analog_clock, scale, 12, 4, 15, lv_color_black(), 10);

    // Seconds indicator (thin red)
    lv_meter_indicator_t *sec_indic = lv_meter_add_needle_line(analog_clock, scale, 2, 
                                      lv_palette_main(LV_PALETTE_RED), -10);

    // Minutes indicator (thicker blue)
    lv_meter_indicator_t *min_indic = lv_meter_add_needle_line(analog_clock, scale, 4, 
                                      lv_palette_main(LV_PALETTE_BLUE), -10);

    // Hours indicator (thickest grey)
    lv_meter_indicator_t *hour_indic = lv_meter_add_needle_line(analog_clock, scale, 6, 
                                       lv_palette_main(LV_PALETTE_GREY), -10);

    // Rotation Button
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text_static(lbl, LV_SYMBOL_REFRESH" ROTATE");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 30, -30);
    lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, disp);

    // Create a timer to update clock every second
    lv_timer_t *timer = lv_timer_create(update_clock, 1000, 
                                        (void*)((uintptr_t)sec_indic | 
                                                ((uintptr_t)min_indic << 32) | 
                                                ((uintptr_t)hour_indic << 64)));
}