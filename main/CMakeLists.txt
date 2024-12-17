idf_component_register(SRCS "main.c" "ui.c" "wifiManager.c" "timeSync.c" "notify.c" "lvglPort.c" "heart.c" "imu.c" "max30100.c"
                       INCLUDE_DIRS "."
                       REQUIRES bt nvs_flash)  