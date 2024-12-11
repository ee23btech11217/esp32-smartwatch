#include "lvgl.h"
#include "esp_log.h"
#include "time.h"
#include <stdio.h>

static const char* TAG = "WatchFace";

// Screen types
typedef enum {
    SCREEN_DIGITAL,
    SCREEN_ANALOG,
    SCREEN_CALENDAR,
    SCREEN_COUNT  // Total number of screens
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
static lv_obj_t *hour_hand = NULL;
static lv_obj_t *minute_hand = NULL;
static lv_obj_t *second_hand = NULL;

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
static void step_update_btn_handler(lv_event_t *e) {
    increment_step_count();
}

// Screen Switch Handler
static void screen_switch_handler(lv_event_t *e) {
    // Move to next screen cyclically
    current_screen = (current_screen + 1) % SCREEN_COUNT;
    
    // Hide all screens
    for (int i = 0; i < SCREEN_COUNT; i++) {
        lv_obj_add_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
    }
    
    // Show current screen
    lv_obj_clear_flag(screens[current_screen], LV_OBJ_FLAG_HIDDEN);
    
    // Update time and content for new screen
    update_time_display();
    update_analog_watch();
}

// Increment step count
void increment_step_count(void) {
    step_count++;
    
    // Update step count label
    char step_str[32];
    snprintf(step_str, sizeof(step_str), "Steps: %d", step_count);
    
    lv_label_set_text(step_label, step_str);
}

// Digital Watch Time Update
void update_time_display(void) {
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
}

// Analog Watch Update
void update_analog_watch(void) {
    if (!hour_hand || !minute_hand || !second_hand) return;
    
    time_t now;
    struct tm timeinfo;
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    // Calculate hand rotations
    int32_t hour_angle = (timeinfo.tm_hour % 12 + timeinfo.tm_min / 60.0) * 30;
    int32_t minute_angle = timeinfo.tm_min * 6;
    int32_t second_angle = timeinfo.tm_sec * 6;
    
    // Rotate hands
    lv_obj_set_style_transform_angle(hour_hand, hour_angle * 10, 0);
    lv_obj_set_style_transform_angle(minute_hand, minute_angle * 10, 0);
    lv_obj_set_style_transform_angle(second_hand, second_angle * 10, 0);
}

// Create Digital Watch Face
void create_digital_watch_face(lv_obj_t *parent) {
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

// Create Analog Watch Face
void create_analog_watch_face(lv_obj_t *parent) {
    // Clock Background (Simple circle)
    lv_obj_t *clock_bg = lv_obj_create(parent);
    lv_obj_set_size(clock_bg, 200, 200);
    lv_obj_center(clock_bg);
    
    // Create hand objects
    static lv_point_t hour_points[] = {{0, 0}, {0, -50}};
    static lv_point_t minute_points[] = {{0, 0}, {0, -70}};
    static lv_point_t second_points[] = {{0, 0}, {0, -90}};
    
    hour_hand = lv_line_create(parent);
    lv_line_set_points(hour_hand, hour_points, 2);
    lv_obj_set_style_line_width(hour_hand, 4, 0);
    lv_obj_set_style_line_color(hour_hand, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_align(hour_hand, LV_ALIGN_CENTER, 0, 0);
    
    minute_hand = lv_line_create(parent);
    lv_line_set_points(minute_hand, minute_points, 2);
    lv_obj_set_style_line_width(minute_hand, 3, 0);
    lv_obj_set_style_line_color(minute_hand, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_align(minute_hand, LV_ALIGN_CENTER, 0, 0);
    
    second_hand = lv_line_create(parent);
    lv_line_set_points(second_hand, second_points, 2);
    lv_obj_set_style_line_width(second_hand, 2, 0);
    lv_obj_set_style_line_color(second_hand, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align(second_hand, LV_ALIGN_CENTER, 0, 0);
}

// Create Calendar Face
void create_calendar_face(lv_obj_t *parent) {
    // Month Display
    month_label = lv_label_create(parent);
    lv_obj_set_pos(month_label, 20, 50);
    lv_label_set_text(month_label, "Calendar");
    
    // Calendar Widget
    calendar = lv_calendar_create(parent);
    lv_obj_set_size(calendar, 200, 200);
    lv_obj_center(calendar);
}

// Time update handler
static void update_time_handler(lv_timer_t *timer) {
    update_time_display();
    update_analog_watch();
}

// Main Watch Face Initialization
void create_watch_face(lv_disp_t *disp) {
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
        
        // Add screen switch event to each screen
        lv_obj_add_event_cb(screens[i], screen_switch_handler, 
            LV_EVENT_CLICKED, NULL);
    }
    
    // Create content for each screen
    create_digital_watch_face(screens[SCREEN_DIGITAL]);
    create_analog_watch_face(screens[SCREEN_ANALOG]);
    create_calendar_face(screens[SCREEN_CALENDAR]);
    
    // Create timer for periodic updates
    lv_timer_create(update_time_handler, 60000, NULL);
    
    // Initial updates
    update_time_display();
    
    ESP_LOGI(TAG, "Multi-Screen Watch Face Initialized");
}