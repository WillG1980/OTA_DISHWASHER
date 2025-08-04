// main.cpp

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <esp_https_ota.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_tls.h>
#include <esp_http_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"
#define UPDATE_URL "https://10.0.0.123/esp/dishwasher/dishwasher.bin"
#define LOGSERVER_IP "10.0.0.123"
#define LOGSERVER_PORT 5000

static const char *TAG = "MAIN";
static int log_sock = -1;

// Custom log function
extern "C" int tcp_log_vprintf(const char *fmt, va_list args) {
    char buf[512];
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    if (log_sock >= 0) {
        send(log_sock, buf, len, 0);
    }
    return vprintf(fmt, args); // Also send to UART
}

void connect_to_wifi() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    esp_wifi_connect();
    ESP_LOGI(TAG, "Connecting to WiFi...");

    // Wait for connection
    vTaskDelay(pdMS_TO_TICKS(5000));
}

void setup_log_socket() {
    struct sockaddr_in dest_addr = {};
    dest_addr.sin_addr.s_addr = inet_addr(LOGSERVER_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(LOGSERVER_PORT);

    log_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (log_sock < 0) {
        ESP_LOGE(TAG, "Unable to create log socket: errno %d", errno);
        return;
    }

    if (connect(log_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
        ESP_LOGE(TAG, "Unable to connect to log server: errno %d", errno);
        close(log_sock);
        log_sock = -1;
        return;
    }

    ESP_LOGI(TAG, "Connected to log server at %s:%d", LOGSERVER_IP, LOGSERVER_PORT);
    esp_log_set_vprintf(tcp_log_vprintf);
}

void ota_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting OTA update from %s", UPDATE_URL);

    esp_http_client_config_t config = {
        .url = UPDATE_URL,
        .cert_pem = NULL,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
        .skip_cert_common_name_check = true,
    };

    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA update successful. Rebooting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA update failed");
    }
    vTaskDelete(NULL);
}

"extern \"C\" void app_main() {
    esp_log_level_set("*", ESP_LOG_INFO);
    nvs_flash_init();

    connect_to_wifi();
    setup_log_socket();

    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}"
