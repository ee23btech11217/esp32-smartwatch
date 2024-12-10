#include "freertos/FreeRTOS.h"
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "time.h"

void setup_time_sync()
{
    // Initialize NVS and network
    nvs_flash_init();
    esp_netif_init();

    // Configure SNTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // Set timezone for India (IST)
    setenv("TZ", "IST+5:00", 1);
    tzset();

    // Wait for time sync
    time_t now = 0;
    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry++ < 10)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
    }
}