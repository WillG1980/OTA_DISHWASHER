#include "time_utils.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "esp_sntp.h"

void initialize_sntp_blocking(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // Wait up to 30 seconds for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int max_retries = 30;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < max_retries) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, max_retries);
        vTaskDelay(pdMS_TO_TICKS(1000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year >= (2016 - 1900)) {
        ESP_LOGI(TAG, "Time synchronized successfully.");
    } else {
        ESP_LOGW(TAG, "Time synchronization failed after %d seconds.", max_retries);
    }
}

time_t get_unix_epoch(void)
{
    time_t now;
    time(&now);
    return now;
}

void print_us_time(void)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGW(TAG, "Time not set. Call initialize_sntp_blocking() first.");
        return;
    }

    char am_pm[3];
    int hour = timeinfo.tm_hour;
    if (hour >= 12) {
        strcpy(am_pm, "PM");
        if (hour > 12) hour -= 12;
    } else {
        strcpy(am_pm, "AM");
        if (hour == 0) hour = 12;
    }

    ESP_LOGI(TAG, "Current time: %02d/%02d/%04d %02d:%02d:%02d %s",
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_year + 1900,
             hour,
             timeinfo.tm_min,
             timeinfo.tm_sec,
             am_pm);
}