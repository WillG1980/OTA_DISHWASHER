#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_https_ota.h"
#include "esp_err.h"
#include "esp_crt_bundle.h"
// ---- Update with your firmware URL ----
#define FIRMWARE_URL "https://house.sjcnu.com/esp32/firmware/ota-dishwasher.bin"

#define WIFI_SSID "House619"
#define WIFI_PASS "Wifi6860"

#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t wifi_event_group;
static const char *TAG = "OTA_WIFI";

// Certificate from server (replace with actual or NULL to disable SSL check)
extern const uint8_t server_cert_pem_start[] asm("_binary_server_cert_pem_start");

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Disconnected. Reconnecting...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Connecting to WiFi...");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to WiFi");
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

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
        //esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA update failed: %s", esp_err_to_name(ret));
    }
}