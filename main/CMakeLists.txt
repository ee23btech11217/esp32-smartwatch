idf_component_register(SRCS "esp32_smartwatch.c" "tasks/gui_task.c"
                       INCLUDE_DIRS "." "tasks"
                       REQUIRES driver esp_timer wifi_manager lvgl lvgl_esp32_drivers)