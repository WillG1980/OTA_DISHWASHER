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

static const char FIRMWARE_URL "https://house.sjcnu.com/esp32/firmware/ota-dishwasher.bin"
static const char TAG "Dishwasher"



#include "dishwasher_programs.h"
#include "local_wifi.c"
//#include "local_ota.c"

// ---- Update with your firmware URL ----
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

}