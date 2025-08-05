#include "dishwasher_programs.h"
#include <esp_event.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_tls.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <netdb.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""
#define UPDATE_URL "https://10.0.0.123:3443/esp32/firmware/ota-dishwasher.bin"
#define LOGSERVER_IP "10.0.0.123"
#define LOGSERVER_PORT 5000

static const char *TAG = "OTA-Test";
static int log_sock = -1;

extern const uint8_t
    server_cert_pem_start[] asm("_binary_server_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_server_cert_pem_end");

// Custom log function
extern "C" int tcp_log_vprintf(const char *fmt, va_list args) {
  char buf[512];
  int len = vsnprintf(buf, sizeof(buf), fmt, args);
  if (log_sock >= 0) {
    int sent = send(log_sock, buf, len, 0);
    if (sent < 0) {
      ESP_LOGE(TAG, "Log socket send failed: errno %d", errno);
      // optionally close socket or set log_sock = -1
    } else if (sent < len) {
      ESP_LOGW(TAG, "Partial log sent (%d of %d bytes)", sent, len);
    }
  }
  return vprintf(fmt, args); // Also send to UART
}

/* Global flag for connection event */
static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

static void on_wifi_event(void *arg, esp_event_base_t base, int32_t id,
                          void *data) {
  if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "WiFi disconnected, retrying...");
    esp_wifi_connect();
  } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void connect_to_wifi() {
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();
  s_wifi_event_group = xEventGroupCreate();

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_event, NULL, &instance_got_ip));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {};
  strncpy((char *)wifi_config.sta.ssid, WIFI_SSID,
          sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, WIFI_PASS,
          sizeof(wifi_config.sta.password));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "WiFi startup initiated, waiting for IP...");

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
                                         pdFALSE, pdTRUE, pdMS_TO_TICKS(10000));
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "Connected to AP and got IP");
  } else {
    ESP_LOGE(TAG, "Failed to connect to WiFi within timeout");
  }
}

void setup_log_socket() {
  ESP_LOGI(TAG, "Setting up log socket...");
  struct sockaddr_in dest_addr = {};
  dest_addr.sin_addr.s_addr = inet_addr(LOGSERVER_IP);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(LOGSERVER_PORT);

  log_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (log_sock < 0) {
    ESP_LOGE(TAG, "Unable to create log socket: errno %d", errno);
    return;
  }

  if (connect(log_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) !=
      0) {
    ESP_LOGE(TAG, "Unable to connect to log server: errno %d", errno);
    close(log_sock);
    log_sock = -1;
    return;
  }

  ESP_LOGI(TAG, "Connected to log server at %s:%d", LOGSERVER_IP,
           LOGSERVER_PORT);
  esp_log_set_vprintf(tcp_log_vprintf);
}

void ota_task(void *pvParameter) {
  ESP_LOGI(TAG, "Starting OTA update from %s", UPDATE_URL);

  esp_http_client_config_t http_config = {};
  http_config.url = UPDATE_URL;
  http_config.cert_pem = NULL;
  http_config.timeout_ms = 5000;
  http_config.keep_alive_enable = true;
  http_config.skip_cert_common_name_check = true;
  http_config.cert_pem = (const char *)server_cert_pem_start;

  esp_https_ota_config_t ota_config =
      (esp_https_ota_config_t){.http_config = &http_config};

  ESP_LOGI(TAG, "Calling esp_https_ota...");
  esp_err_t ret = esp_https_ota(&ota_config);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "OTA update successful. Rebooting...");
    esp_restart();
  } else {
    ESP_LOGE(TAG, "OTA update failed: %d", ret);
  }
  vTaskDelete(NULL);
}

extern "C" void app_main() {
  ESP_LOGI(TAG, "App main started");
  ESP_LOGI("WIFI:", "WIFI SSID: %s", WIFI_SSID);
  esp_log_level_set("*", ESP_LOG_INFO);

  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("mbedtls", ESP_LOG_VERBOSE);
  esp_log_level_set("esp_https_ota", ESP_LOG_VERBOSE);

  nvs_flash_init();
  connect_to_wifi();
  setup_log_socket();
  ESP_LOGI(TAG, "Creating OTA task...");
  xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}