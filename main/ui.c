#include "lvgl.h"
#include "esp_log.h"
#include "time.h"
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

static const char *TAG = "WatchFace";

extern bool get_heart_rate_data(float *heart_rate, float *spo2);

typedef enum {
    SCREEN_DIGITAL,
    SCREEN_ANALOG,
    SCREEN_COUNT
} screen_type_t;

typedef struct {
    screen_type_t current_screen;
    lv_obj_t *screens[SCREEN_COUNT];

    // Digital watch components
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *step_label;
    lv_obj_t *heart_rate_label;
    lv_obj_t *spo2_label;

    // Analog watch components
    lv_obj_t *clock_bg;
    lv_obj_t *hour_hand;
    lv_obj_t *minute_hand;
    lv_obj_t *second_hand;
    lv_obj_t *center_circle;
    lv_point_t hour_points[2];
    lv_point_t minute_points[2];
    lv_point_t second_points[2];

    // State tracking
    uint32_t last_touch_time;
    int last_second;
    int last_minute;
    int last_hour;
} watch_ui_t;

static watch_ui_t watch_ui = {0};

static const uint32_t SCREEN_SWITCH_TIMEOUT = 500;
static const int CLOCK_CENTER_X = 113;
static const int CLOCK_CENTER_Y = 148;
static const int HOUR_HAND_LENGTH = 60;
static const int MINUTE_HAND_LENGTH = 80;
static const int SECOND_HAND_LENGTH = 90;

void update_analog_watch(void) {
    // Quick exit if no hand components exist
    if (!watch_ui.hour_hand || !watch_ui.minute_hand || !watch_ui.second_hand)
        return;

    // Get current time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);

    // Optimize updates by checking if time has actually changed
    if (timeinfo->tm_sec == watch_ui.last_second && 
        timeinfo->tm_min == watch_ui.last_minute && 
        timeinfo->tm_hour == watch_ui.last_hour) {
        return;
    }

    // Update tracking
    watch_ui.last_second = timeinfo->tm_sec;
    watch_ui.last_minute = timeinfo->tm_min;
    watch_ui.last_hour = timeinfo->tm_hour;

    // Precise rotation calculations
    float hour_rot = (timeinfo->tm_hour % 12 + timeinfo->tm_min / 60.0) * 30.0;
    float minute_rot = (timeinfo->tm_min + timeinfo->tm_sec / 60.0) * 6.0;
    float second_rot = timeinfo->tm_sec * 6.0;

    // Update hand points with trigonometric calculations
    watch_ui.hour_points[0].x = CLOCK_CENTER_X;
    watch_ui.hour_points[0].y = CLOCK_CENTER_Y;
    watch_ui.hour_points[1].x = CLOCK_CENTER_X + HOUR_HAND_LENGTH * sin(hour_rot * M_PI / 180);
    watch_ui.hour_points[1].y = CLOCK_CENTER_Y - HOUR_HAND_LENGTH * cos(hour_rot * M_PI / 180);

    watch_ui.minute_points[0].x = CLOCK_CENTER_X;
    watch_ui.minute_points[0].y = CLOCK_CENTER_Y;
    watch_ui.minute_points[1].x = CLOCK_CENTER_X + MINUTE_HAND_LENGTH * sin(minute_rot * M_PI / 180);
    watch_ui.minute_points[1].y = CLOCK_CENTER_Y - MINUTE_HAND_LENGTH * cos(minute_rot * M_PI / 180);

    watch_ui.second_points[0].x = CLOCK_CENTER_X;
    watch_ui.second_points[0].y = CLOCK_CENTER_Y;
    watch_ui.second_points[1].x = CLOCK_CENTER_X + SECOND_HAND_LENGTH * sin(second_rot * M_PI / 180);
    watch_ui.second_points[1].y = CLOCK_CENTER_Y - SECOND_HAND_LENGTH * cos(second_rot * M_PI / 180);

    // Efficient line updates
    lv_line_set_points(watch_ui.hour_hand, watch_ui.hour_points, 2);
    lv_line_set_points(watch_ui.minute_hand, watch_ui.minute_points, 2);
    lv_line_set_points(watch_ui.second_hand, watch_ui.second_points, 2);
}

void update_time_display(void) {
    // Quick exit if labels not initialized
    if (!watch_ui.time_label || !watch_ui.date_label)
        return;

    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    // Only update if time has changed
    static char last_time_str[6] = {0};
    static char last_date_str[32] = {0};

    char time_str[6], date_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);
    strftime(date_str, sizeof(date_str), "%b %d, %Y", timeinfo);

    // Compare and update only if changed
    if (strcmp(time_str, last_time_str) != 0) {
        lv_label_set_text(watch_ui.time_label, time_str);
        strcpy(last_time_str, time_str);
    }

    if (strcmp(date_str, last_date_str) != 0) {
        lv_label_set_text(watch_ui.date_label, date_str);
        strcpy(last_date_str, date_str);
    }

    // Update heart rate and SpO2
    float heart_rate, spo2;
    if (get_heart_rate_data(&heart_rate, &spo2)) {
        static float last_heart_rate = 0.0f;
        static float last_spo2 = 0.0f;

        // Only update if values have changed significantly
        if (fabs(heart_rate - last_heart_rate) > 0.5f || fabs(spo2 - last_spo2) > 0.5f) {
            lv_label_set_text_fmt(watch_ui.heart_rate_label, "HR: %.1f BPM", heart_rate);
            lv_label_set_text_fmt(watch_ui.spo2_label, "SpO2: %.1f%%", spo2);
            
            last_heart_rate = heart_rate;
            last_spo2 = spo2;
        }
    }
}

// Screen switch handler with improved debounce
static void screen_switch_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    uint32_t current_time = lv_tick_get();

    if (code == LV_EVENT_RELEASED &&
        (current_time - watch_ui.last_touch_time >= SCREEN_SWITCH_TIMEOUT)) {
        
        // Cycle through screens
        watch_ui.current_screen = (watch_ui.current_screen + 1) % SCREEN_COUNT;

        // Efficiently manage screen visibility
        for (int i = 0; i < SCREEN_COUNT; i++) {
            if (i == watch_ui.current_screen) {
                lv_obj_clear_flag(watch_ui.screens[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(watch_ui.screens[i], LV_OBJ_FLAG_HIDDEN);
            }
        }

        // Force update of current screen
        update_time_display();
        update_analog_watch();

        watch_ui.last_touch_time = current_time;
    }
}

// Create digital watch face with minimal allocations
void create_digital_watch_face(lv_obj_t *parent)
{
    watch_ui.time_label = lv_label_create(parent);
    lv_obj_set_pos(watch_ui.time_label, 20, 50);
    lv_obj_set_style_text_font(watch_ui.time_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(watch_ui.time_label, lv_palette_main(LV_PALETTE_BLUE), 0);

    watch_ui.date_label = lv_label_create(parent);
    lv_obj_set_pos(watch_ui.date_label, 20, 100);

    watch_ui.step_label = lv_label_create(parent);
    lv_obj_set_pos(watch_ui.step_label, 20, 170);
    lv_label_set_text(watch_ui.step_label, "Steps: 0");

    watch_ui.heart_rate_label = lv_label_create(parent);
    lv_obj_set_pos(watch_ui.heart_rate_label, 20, 220);

    watch_ui.spo2_label = lv_label_create(parent);
    lv_obj_set_pos(watch_ui.spo2_label, 20, 270);
}

// Enhanced analog watch face with improved aesthetics
void create_analog_watch_face(lv_obj_t *parent)
{
    // Validate parent screen
    if (!parent)
    {
        ESP_LOGE(TAG, "Invalid parent screen in create_analog_watch_face");
        return;
    }

    // Create clock background with a more refined look
    watch_ui.clock_bg = lv_obj_create(parent);
    if (!watch_ui.clock_bg)
    {
        ESP_LOGE(TAG, "Failed to create clock background");
        return;
    }
    lv_obj_set_size(watch_ui.clock_bg, 200, 200);
    lv_obj_center(watch_ui.clock_bg);

    // Set background style
    lv_obj_set_style_bg_color(watch_ui.clock_bg, lv_color_make(240, 240, 240), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(watch_ui.clock_bg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(watch_ui.clock_bg, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(watch_ui.clock_bg, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_radius(watch_ui.clock_bg, 100, LV_PART_MAIN);

    // Create hour markings with enhanced style
    for (int i = 0; i < 12; i++)
    {
        float angle = i * 30.0 * (M_PI / 180);
        lv_obj_t *mark = lv_line_create(parent);
        if (!mark)
        {
            ESP_LOGE(TAG, "Failed to create hour marking %d", i);
            continue;
        }
        lv_point_t points[2];

        // Outer circle points for longer markings
        points[0].x = CLOCK_CENTER_X + sin(angle) * 110;
        points[0].y = CLOCK_CENTER_Y - cos(angle) * 110;

        // Inner circle points
        points[1].x = CLOCK_CENTER_X + sin(angle) * 100;
        points[1].y = CLOCK_CENTER_Y - cos(angle) * 100;

        lv_line_set_points(mark, points, 2);

        // Thicker markings for 12, 3, 6, 9
        if (i % 3 == 0)
        {
            lv_obj_set_style_line_width(mark, 4, LV_PART_MAIN);
            lv_obj_set_style_line_color(mark, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_line_width(mark, 2, LV_PART_MAIN);
            lv_obj_set_style_line_color(mark, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_PART_MAIN);
        }
    }

    // Add hour numbers with improved styling
    const char *hour_labels[] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};

    for (int i = 0; i < 12; i++)
    {
        float angle = i * 30.0 * (M_PI / 180);

        // Create label for hour number
        lv_obj_t *number_label = lv_label_create(parent);
        if (!number_label)
        {
            ESP_LOGE(TAG, "Failed to create hour number label %d", i);
            continue;
        }

        // Position calculation
        int radius = 85; // Slightly inside the hour markings
        int x = CLOCK_CENTER_X + sin(angle) * radius;
        int y = CLOCK_CENTER_Y - cos(angle) * radius;

        // Set label text and position
        lv_label_set_text(number_label, hour_labels[i]);
        lv_obj_set_pos(number_label,
                       x - (i == 0 || i == 10 ? 15 : 10),
                       y - 10);

        // Style the hour numbers
        lv_obj_set_style_text_color(number_label, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_PART_MAIN);
        lv_obj_set_style_text_font(number_label, &lv_font_montserrat_14, 0);
    }

    // Initialize hand points
    watch_ui.hour_points[0].x = CLOCK_CENTER_X;
    watch_ui.hour_points[0].y = CLOCK_CENTER_Y;
    watch_ui.hour_points[1].x = CLOCK_CENTER_X;
    watch_ui.hour_points[1].y = CLOCK_CENTER_Y - HOUR_HAND_LENGTH;

    watch_ui.minute_points[0].x = CLOCK_CENTER_X;
    watch_ui.minute_points[0].y = CLOCK_CENTER_Y;
    watch_ui.minute_points[1].x = CLOCK_CENTER_X;
    watch_ui.minute_points[1].y = CLOCK_CENTER_Y - MINUTE_HAND_LENGTH;

    watch_ui.second_points[0].x = CLOCK_CENTER_X;
    watch_ui.second_points[0].y = CLOCK_CENTER_Y;
    watch_ui.second_points[1].x = CLOCK_CENTER_X;
    watch_ui.second_points[1].y = CLOCK_CENTER_Y - SECOND_HAND_LENGTH;

    // Create clock hands with improved styling
    watch_ui.hour_hand = lv_line_create(parent);
    lv_line_set_points(watch_ui.hour_hand, watch_ui.hour_points, 2);
    lv_obj_set_style_line_width(watch_ui.hour_hand, 6, LV_PART_MAIN);
    lv_obj_set_style_line_color(watch_ui.hour_hand, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(watch_ui.hour_hand, true, LV_PART_MAIN);

    watch_ui.minute_hand = lv_line_create(parent);
    lv_line_set_points(watch_ui.minute_hand, watch_ui.minute_points, 2);
    lv_obj_set_style_line_width(watch_ui.minute_hand, 4, LV_PART_MAIN);
    lv_obj_set_style_line_color(watch_ui.minute_hand, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(watch_ui.minute_hand, true, LV_PART_MAIN);

    watch_ui.second_hand = lv_line_create(parent);
    lv_line_set_points(watch_ui.second_hand, watch_ui.second_points, 2);
    lv_obj_set_style_line_width(watch_ui.second_hand, 2, LV_PART_MAIN);
    lv_obj_set_style_line_color(watch_ui.second_hand, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(watch_ui.second_hand, true, LV_PART_MAIN);

    // Enhanced center circle
    watch_ui.center_circle = lv_obj_create(parent);
    lv_obj_set_size(watch_ui.center_circle, 12, 12);
    lv_obj_set_pos(watch_ui.center_circle,
                   CLOCK_CENTER_X - 6,
                   CLOCK_CENTER_Y - 6);
    lv_obj_set_style_bg_color(watch_ui.center_circle, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_radius(watch_ui.center_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(watch_ui.center_circle, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(watch_ui.center_circle, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_PART_MAIN);
}

// Periodic update handler with reduced frequency
static void update_time_handler(lv_timer_t *timer) {
    update_time_display();
    update_analog_watch();
}

// Create watch face with minimal redraws
void create_watch_face(lv_disp_t *disp, float heart_rate, float spo2) {
    // Prevent multiple initializations
    static bool initialized = false;
    if (initialized) {
        return;
    }
    initialized = true;

    lv_obj_t *scr = lv_disp_get_scr_act(disp);

    // Initialize screens
    for (int i = 0; i < SCREEN_COUNT; i++) {
        watch_ui.screens[i] = lv_obj_create(scr);
        lv_obj_set_size(watch_ui.screens[i],
                        lv_disp_get_hor_res(disp),
                        lv_disp_get_ver_res(disp));

        // Hide all but digital screen initially
        if (i != SCREEN_DIGITAL) {
            lv_obj_add_flag(watch_ui.screens[i], LV_OBJ_FLAG_HIDDEN);
        }

        // Add touch event for screen switching
        lv_obj_add_event_cb(watch_ui.screens[i], screen_switch_handler,
                            LV_EVENT_RELEASED, NULL);
    }

    // Create watch faces for different screens
    create_digital_watch_face(watch_ui.screens[SCREEN_DIGITAL]);
    create_analog_watch_face(watch_ui.screens[SCREEN_ANALOG]);

    // Manually set initial heart rate and SpO2 labels
    if (watch_ui.heart_rate_label && watch_ui.spo2_label) {
        lv_label_set_text_fmt(watch_ui.heart_rate_label, "HR: %.1f BPM", heart_rate);
        lv_label_set_text_fmt(watch_ui.spo2_label, "SpO2: %.1f%%", spo2);
    }

    // Create timer for periodic updates (every 2 seconds to reduce overhead)
    lv_timer_create(update_time_handler, 2000, NULL);

    // Initial time and watch display
    update_time_display();
    update_analog_watch();
}

