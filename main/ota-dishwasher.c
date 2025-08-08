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
#include "driver/gpio.h"

#include "dishwasher_programs.h"
#include "local_wifi.c"
#include "local_ota.c"
#include "buttons.h"
#include "analog.h"
void _init_setup(){
    wifi_init_sta();
    init_switchesandleds();
    init_adc();

    //setup background monitoring tasks
    xTaskCreate(monitor_task_buttons, "monitor_task_buttons", 2048, NULL, 5, NULL); // monitor buttons, set flags as needed
    xTaskCreate(monitor_task_temperature, "monitor_task_temperature", 2048, NULL, 5, NULL); //monitor temperatures
    xTaskCreate(update_published_status, "update_published_status", 2048, NULL, 5, NULL); //publish status
    //xTaskCreate(sample_analog_inputs_task, "sample_analog_inputs_task", 4096, NULL, 5, NULL);
}
void monitor_task_buttons(){}
void monitor_task_temperature(){}
void update_published_status(){}


void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

    ESP_LOGI("Dishwasher","Version: %s \nFirmware Update:%s\n",VERSION,FIRMWARE_URL);
        printf( "\nTotal program count: %d\n",NUM_PROGRAMS);

    for (int i=0;i<NUM_PROGRAMS;i++){
        Program_Entry Program=Programs[i];
        printf("Program Name: %s\n",Program.name);
    }  

}