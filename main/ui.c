#include "lvgl.h"
#include "esp_log.h"
#include "time.h"
#include <stdio.h>
#include <math.h>

static const char *TAG = "WatchFace";

// Screen types
typedef enum
{
    SCREEN_DIGITAL,
    SCREEN_ANALOG,
    SCREEN_CALENDAR,
    SCREEN_COUNT 
} screen_type_t;

// Global state
static screen_type_t current_screen = SCREEN_DIGITAL;
static lv_obj_t *screens[SCREEN_COUNT];

// Digital Watch Face Components
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *battery_indicator = NULL;
static lv_obj_t *step_label = NULL;
static lv_obj_t *step_update_btn = NULL;

// Step count state
static int step_count = 0;

// Analog Watch Face Components
static lv_obj_t *clock_bg = NULL;
static lv_obj_t *hour_hand = NULL;
static lv_obj_t *minute_hand = NULL;
static lv_obj_t *second_hand = NULL;

// Screen Switch State
static uint32_t last_touch_time = 0;
static const uint32_t SCREEN_SWITCH_TIMEOUT = 5000; // 5 seconds timeout

// Calendar Components
static lv_obj_t *month_label = NULL;
static lv_obj_t *calendar = NULL;

// Function Prototypes
void update_time_display(void);
void update_analog_watch(void);
void create_digital_watch_face(lv_obj_t *parent);
void create_analog_watch_face(lv_obj_t *parent);
void create_calendar_face(lv_obj_t *parent);
void increment_step_count(void);

// Step Update Button Handler
static void step_update_btn_handler(lv_event_t *e)
{
    ESP_LOGI(TAG, "Step update button clicked");
    increment_step_count();
}

static void screen_switch_handler(lv_event_t *e)
{
    // Get event code to differentiate between different touch interactions
    lv_event_code_t code = lv_event_get_code(e);

    // Get current time
    uint32_t current_time = lv_tick_get();

    // Check if enough time has passed since last touch and if it's a touch release event
    if (code == LV_EVENT_RELEASED &&
        (current_time - last_touch_time >= SCREEN_SWITCH_TIMEOUT))
    {

        // Move to next screen cyclically
        current_screen = (current_screen + 1) % SCREEN_COUNT;
        ESP_LOGI(TAG, "Screen switched to %d", current_screen);

        // Hide all screens
        for (int i = 0; i < SCREEN_COUNT; i++)
        {
            lv_obj_add_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
        }

        // Show current screen
        lv_obj_clear_flag(screens[current_screen], LV_OBJ_FLAG_HIDDEN);

        // Update time and content for new screen
        update_time_display();
        update_analog_watch();

        // Reset touch time
        last_touch_time = current_time;
    }
    else if (code == LV_EVENT_PRESSING)
    {
        // Optional: Add logging for pressing events if needed
        ESP_LOGD(TAG, "Screen being touched");
    }
}

// Increment step count
void increment_step_count(void)
{
    step_count++;

    // Update step count label
    char step_str[32];
    snprintf(step_str, sizeof(step_str), "Steps: %d", step_count);

    lv_label_set_text(step_label, step_str);
    ESP_LOGI(TAG, "Step count incremented to %d", step_count);
}

// Digital Watch Time Update
void update_time_display(void)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    // Time format: HH:MM
    char time_str[6];
    strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);
    lv_label_set_text(time_label, time_str);

    // Date format: Day, Month Date
    char date_str[32];
    strftime(date_str, sizeof(date_str), "%b %d, %Y", &timeinfo);
    lv_label_set_text(date_label, date_str);

    ESP_LOGD(TAG, "Time updated: %s, Date: %s", time_str, date_str);
}

// Analog Watch Update with Improved Precision
void update_analog_watch(void)
{
    if (!hour_hand || !minute_hand || !second_hand)
    {
        ESP_LOGE(TAG, "Analog watch hands not initialized");
        return;
    }

    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    // More precise hand rotations
    // Hours: 360 degrees / 12 hours = 30 degrees per hour
    // Minutes contribute to hour hand movement
    float hour_rotation = (timeinfo.tm_hour % 12 + timeinfo.tm_min / 60.0) * 30.0;

    // Minutes: 360 degrees / 60 minutes = 6 degrees per minute
    float minute_rotation = (timeinfo.tm_min + timeinfo.tm_sec / 60.0) * 6.0;

    // Seconds: 360 degrees / 60 seconds = 6 degrees per second
    float second_rotation = timeinfo.tm_sec * 6.0;

    // Update hand rotations with more precision
    lv_obj_set_style_transform_angle(hour_hand, (int32_t)(hour_rotation * 10), 0);
    lv_obj_set_style_transform_angle(minute_hand, (int32_t)(minute_rotation * 10), 0);
    lv_obj_set_style_transform_angle(second_hand, (int32_t)(second_rotation * 10), 0);

    ESP_LOGD(TAG, "Analog watch updated - Hour: %.2f, Minute: %.2f, Second: %.2f",
             hour_rotation, minute_rotation, second_rotation);
}

// Create Digital Watch Face
void create_digital_watch_face(lv_obj_t *parent)
{
    ESP_LOGI(TAG, "Creating Digital Watch Face");

    // Time Display
    time_label = lv_label_create(parent);
    lv_obj_set_pos(time_label, 20, 50);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(time_label, lv_palette_main(LV_PALETTE_BLUE), 0);

    // Date Display
    date_label = lv_label_create(parent);
    lv_obj_set_pos(date_label, 20, 100);

    // Battery Indicator
    battery_indicator = lv_label_create(parent);
    lv_obj_set_pos(battery_indicator, 20, 150);
    lv_label_set_text(battery_indicator, "Battery: 80%");

    // Step Counter
    step_label = lv_label_create(parent);
    lv_obj_set_pos(step_label, 20, 170);
    lv_label_set_text(step_label, "Steps: 0");

    // Step Update Button
    step_update_btn = lv_btn_create(parent);
    lv_obj_set_pos(step_update_btn, 20, 200);
    lv_obj_set_size(step_update_btn, 100, 40);

    // Button Label
    lv_obj_t *btn_label = lv_label_create(step_update_btn);
    lv_label_set_text(btn_label, "Update Steps");
    lv_obj_center(btn_label);

    // Add click event to button
    lv_obj_add_event_cb(step_update_btn, step_update_btn_handler, LV_EVENT_CLICKED, NULL);
}

// Create Analog Watch Face with Improved Visualization
void create_analog_watch_face(lv_obj_t *parent)
{
    ESP_LOGI(TAG, "Creating Analog Watch Face");

    // Clock Background with more styling
    clock_bg = lv_obj_create(parent);
    lv_obj_set_size(clock_bg, 200, 200);
    lv_obj_center(clock_bg);
    lv_obj_set_style_bg_color(clock_bg, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_border_color(clock_bg, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_border_width(clock_bg, 2, 0);

    // Create hour marks
    for (int i = 0; i < 12; i++)
    {
        float angle = i * 30;
        float radian = angle * (3.14159 / 180);

        lv_obj_t *mark = lv_line_create(parent);
        lv_point_t points[2];
        points[0].x = 100 + sin(radian) * 90;
        points[0].y = 100 - cos(radian) * 90;
        points[1].x = 100 + sin(radian) * 95;
        points[1].y = 100 - cos(radian) * 95;

        lv_line_set_points(mark, points, 2);
        lv_obj_set_style_line_width(mark, 2, 0);
        lv_obj_set_style_line_color(mark, lv_palette_main(LV_PALETTE_GREY), 0);
    }

    // Create hand objects with origin at center
    static lv_point_t hour_points[] = {{100, 100}, {100, 50}};
    static lv_point_t minute_points[] = {{100, 100}, {100, 30}};
    static lv_point_t second_points[] = {{100, 100}, {100, 20}};

    hour_hand = lv_line_create(parent);
    lv_line_set_points(hour_hand, hour_points, 2);
    lv_obj_set_style_line_width(hour_hand, 5, 0);
    lv_obj_set_style_line_color(hour_hand, lv_palette_main(LV_PALETTE_BLUE), 0);

    minute_hand = lv_line_create(parent);
    lv_line_set_points(minute_hand, minute_points, 2);
    lv_obj_set_style_line_width(minute_hand, 3, 0);
    lv_obj_set_style_line_color(minute_hand, lv_palette_main(LV_PALETTE_GREEN), 0);

    second_hand = lv_line_create(parent);
    lv_line_set_points(second_hand, second_points, 2);
    lv_obj_set_style_line_width(second_hand, 1, 0);
    lv_obj_set_style_line_color(second_hand, lv_palette_main(LV_PALETTE_RED), 0);
}

void create_calendar_face(lv_obj_t *parent)
{
    ESP_LOGI(TAG, "Creating Calendar Face");

    // Month Display
    month_label = lv_label_create(parent);
    lv_obj_set_pos(month_label, 20, 50);

    // Calendar Widget
    calendar = lv_calendar_create(parent);
    lv_obj_set_size(calendar, 240, 240);
    lv_obj_center(calendar);

    // Set current date
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Set today's date using separate arguments
    lv_calendar_set_today_date(
        calendar,
        timeinfo.tm_year + 1900, // year
        timeinfo.tm_mon + 1,     // month (1-12)
        timeinfo.tm_mday         // day
    );

    // Set showed date (only year and month)
    lv_calendar_set_showed_date(
        calendar,
        timeinfo.tm_year + 1900, // year
        timeinfo.tm_mon + 1      // month (1-12)
    );

    // Update month label
    char month_str[32];
    strftime(month_str, sizeof(month_str), "%B %Y", &timeinfo);
    lv_label_set_text(month_label, month_str);
}

// Time update handler
static void update_time_handler(lv_timer_t *timer)
{
    update_time_display();
    update_analog_watch();

    ESP_LOGD(TAG, "Periodic time update triggered");
}

// Main Watch Face Initialization
void create_watch_face(lv_disp_t *disp)
{
ESP_LOGI(TAG, "Creating Multi-Screen Watch Face");
    
    // Get active screen
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    
    // Create screens
    for (int i = 0; i < SCREEN_COUNT; i++) {
        screens[i] = lv_obj_create(scr);
        
        // Set full screen size for each
        lv_obj_set_size(screens[i], 
            lv_disp_get_hor_res(disp), 
            lv_disp_get_ver_res(disp));
        
        // Initially hide all screens except the first
        if (i != SCREEN_DIGITAL) {
            lv_obj_add_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
        }
        
        // Add multiple event types for screen switching
        lv_obj_add_event_cb(screens[i], screen_switch_handler, 
            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(screens[i], screen_switch_handler, 
            LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(screens[i], screen_switch_handler, 
            LV_EVENT_PRESSING, NULL);
    }

    // Create content for each screen
    create_digital_watch_face(screens[SCREEN_DIGITAL]);
    create_analog_watch_face(screens[SCREEN_ANALOG]);
    create_calendar_face(screens[SCREEN_CALENDAR]);

    // Create timer for periodic updates (every minute)
    lv_timer_create(update_time_handler, 60000, NULL);

    // Initial updates
    update_time_display();
    update_analog_watch();

    ESP_LOGI(TAG, "Multi-Screen Watch Face Initialized");
}