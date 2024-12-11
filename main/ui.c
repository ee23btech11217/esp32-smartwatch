#include "lvgl.h"
#include "esp_log.h"
#include "time.h"
#include <stdio.h>

static const char* TAG = "WatchFace";

// Watch face UI components
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *battery_indicator = NULL;
static lv_obj_t *step_label = NULL;
static lv_obj_t *weather_icon = NULL;

// State variables
static int current_battery_level = 75;  // Simulated battery level
static int step_count = 0;

// Function prototypes
void update_time_display(void);
void update_battery_indicator(void);
void increment_step_count(void);

// Time update event handler
static void update_time_handler(lv_timer_t *timer) {
    update_time_display();
    update_battery_indicator();
}

// Step count simulation event handler
static void step_timer_handler(lv_timer_t *timer) {
    increment_step_count();
}

// Update time display
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

// Simulated battery level management
void update_battery_indicator(void) {
    // Simulate battery drain
    current_battery_level = (current_battery_level > 0) ? current_battery_level - 1 : 0;
    
    // Update battery indicator color based on level
    lv_color_t battery_color;
    if (current_battery_level > 50) {
        battery_color = lv_palette_main(LV_PALETTE_GREEN);
    } else if (current_battery_level > 20) {
        battery_color = lv_palette_main(LV_PALETTE_ORANGE);
    } else {
        battery_color = lv_palette_main(LV_PALETTE_RED);
    }
    
    // Create battery indicator using a rectangle
    char battery_text[20];
    snprintf(battery_text, sizeof(battery_text), "Battery: %d%%", current_battery_level);
    lv_label_set_text(battery_indicator, battery_text);
    lv_obj_set_style_text_color(battery_indicator, battery_color, 0);
}

// Simulate step count increment
// Convert integer to string manually
void int_to_str(int num, char* str, int max_len) {
    int i = 0;
    int is_negative = 0;
    
    // Handle 0 as a special case
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Convert digits
    while (num > 0 && i < max_len - 1) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        str[i++] = '-';
    }
    
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    
    str[i] = '\0';
}

void increment_step_count(void) {
    step_count++;
    
    // Update step count label
    char step_str[16] = "Steps: ";
    char num_str[11];  // Max 10 digits for 32-bit int + null terminator
    
    int_to_str(step_count, num_str, sizeof(num_str));
    
    // Concatenate "Steps: " with the number
    int base_len = 7;  // Length of "Steps: "
    for (int i = 0; num_str[i] != '\0' && base_len < sizeof(step_str) - 1; i++) {
        step_str[base_len++] = num_str[i];
    }
    step_str[base_len] = '\0';
    
    lv_label_set_text(step_label, step_str);
}

// Weather icon tap handler
static void weather_tap_handler(lv_event_t *e) {
    // Placeholder for weather update logic
    ESP_LOGI(TAG, "Weather icon tapped - trigger weather refresh");
    // In a real implementation, this would fetch latest weather data
}

// Create watch face UI
void create_watch_face(lv_disp_t *disp) {
    ESP_LOGI(TAG, "Creating Watch Face UI");
    
    // Get active screen
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    
    // Get screen dimensions
    lv_coord_t screen_width = lv_disp_get_hor_res(disp);
    lv_coord_t screen_height = lv_disp_get_ver_res(disp);
    
    // Time Display
    time_label = lv_label_create(scr);
    lv_obj_set_pos(time_label, 20, 50);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(time_label, lv_palette_main(LV_PALETTE_BLUE), 0);
    
    // Date Display
    date_label = lv_label_create(scr);
    lv_obj_set_pos(date_label, 20, 100);
    lv_obj_set_style_text_color(date_label, lv_palette_main(LV_PALETTE_GREY), 0);
    
    // Battery Indicator (changed to label for better visibility)
    battery_indicator = lv_label_create(scr);
    lv_obj_set_pos(battery_indicator, 20, 150);
    
    // Step Counter
    step_label = lv_label_create(scr);
    lv_obj_set_pos(step_label, 20, 170);
    
    // Weather Icon (Simulated)
    weather_icon = lv_label_create(scr);
    lv_label_set_text(weather_icon, "Sunny");  // Sunny as placeholder
    lv_obj_set_pos(weather_icon, screen_width - 50, 50);  // Positioned from right side
    lv_obj_add_event_cb(weather_icon, weather_tap_handler, LV_EVENT_CLICKED, NULL);
    
    // Create timers for periodic updates
    lv_timer_create(update_time_handler, 60000, NULL);  // Update time every minute
    lv_timer_create(step_timer_handler, 10000, NULL);   // Simulate step increments
    
    // Initial updates
    update_time_display();
    update_battery_indicator();
    
    ESP_LOGI(TAG, "Watch Face UI Initialization Complete");
}