#ifndef LOCAL_WIFI_H
#define LOCAL_WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

// Event bits
#define WIFI_CONNECTED_BIT BIT0

// Connection timeout in ticks
#define WIFI_FAIL_TIMEOUT pdMS_TO_TICKS(5000)  // 5 seconds

// Wi-Fi credentials
#define WIFI_SSID_REAL   "House619"
#define WIFI_PASS_REAL   "Wifi6860"
#define WIFI_SSID_WOKWI  "Wokwi-GUEST"
#define WIFI_PASS_WOKWI  ""

// Tag for logging
extern const char *TAG;

/**
 * @brief Initialize Wi-Fi in STA mode, attempt to connect to real or simulator SSID.
 * 
 * Tries WIFI_SSID_REAL first, then WIFI_SSID_WOKWI as a fallback.
 * Sets internal flag to indicate if running on simulator.
 */
void wifi_init_sta(void);

/**
 * @brief Check if currently connected to a Wi-Fi network.
 * 
 * @return true if connected, false otherwise.
 */
bool is_connected(void);

/**
 * @brief Check if the device is running in simulator mode (Wokwi).
 * 
 * @return true if using simulator SSID, false if connected to real Wi-Fi.
 */
bool is_simulator(void);

#endif // LOCAL_WIFI_H