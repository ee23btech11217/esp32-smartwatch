#include <stdio.h>
#include <time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"

static const char *TAG = "time_sync";

void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // Set timezone
    setenv("TZ", "IST-5:30", 1);
    tzset();
}