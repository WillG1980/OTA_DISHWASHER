#include <string.h>
#include "esp_https_ota.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"

extern const char *TAG;
extern const char *FIRMWARE_URL;


void _get_ota() {
    esp_http_client_config_t config = {
        .url = FIRMWARE_URL,
        .crt_bundle_attach = esp_crt_bundle_attach
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    ESP_LOGI(TAG, "Starting OTA update...");
    esp_err_t ret = esp_https_ota(&ota_config);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA update successful. Rebooting...");
        // esp_restart(); // Uncomment this in production
    } else {
        ESP_LOGE(TAG, "OTA update failed: %s", esp_err_to_name(ret));
    }
}