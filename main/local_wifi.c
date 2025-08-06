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
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "dishwasher_programs.h"
#include "local_wifi.c"
#include "local_ota.c"


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_TIMEOUT pdMS_TO_TICKS(5000)  // 5 seconds to try

#define WIFI_SSID_REAL "House619"
#define WIFI_PASS_REAL "Wifi6860"
#define WIFI_SSID_WOKWI "Wokwi-GUEST"
#define WIFI_PASS_WOKWI ""

static EventGroupHandle_t wifi_event_group;
static const char *TAG = "WIFI_INIT";

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

static bool try_connect_wifi(const char *ssid, const char *pass) {
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, WIFI_FAIL_TIMEOUT);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi: %s", ssid);
        return true;
    } else {
        ESP_LOGW(TAG, "Connection to SSID %s failed", ssid);
        return false;
    }
}
void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    // First try real WiFi
    if (!try_connect_wifi(WIFI_SSID_REAL, WIFI_PASS_REAL)) {
        ESP_LOGW(TAG, "Falling back to Wokwi network...");
        try_connect_wifi(WIFI_SSID_WOKWI, WIFI_PASS_WOKWI);
    }
    ESP_LOGI("Wifi","Wifi connection ended");
}
static EventGroupHandle_t wifi_event_group;
