#include "driver/gpio.h"
#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include <string.h>


#include "analog.h"
#include "buttons.h"
#include "dishwasher_programs.h"
#include "local_ota.c"
#include "local_wifi.c"
#include "local_time.h"

#define __TAG__ "OTA_DISHWASHER"

// prototyping functions
status_struct ActiveStatus;

void monitor_task_buttons();
void monitor_task_temperature();
void update_published_status();
void init_status();

void _init_setup() {
  wifi_init_sta();
  init_switchesandleds();
  init_adc();
  init_status();
  // setup background monitoring tasks
  xTaskCreate(monitor_task_button, "monitor_task_buttons", 2048, NULL, 5,
              NULL); // monitor buttons, set flags as needed
  xTaskCreate(monitor_task_temperature, "monitor_task_temperature", 2048, NULL,
              5, NULL); // monitor temperatures
  xTaskCreate(update_published_status, "update_published_status", 2048, NULL, 5,
              NULL); // publish status
  // xTaskCreate(sample_analog_inputs_task, "sample_analog_inputs_task", 4096,
  // NULL, 5, NULL);

  for (int t = 60; t > 0; t--) {
    _LOG_I("Waiting for wifi, %d seconds remaining", t);
    if (is_connected()) {
      _LOG_I("Connected to Wifi");
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
};
void monitor_task_buttons() {}
void monitor_task_temperature() {}
void update_published_status() {
  while(true){
    printf("Status update: State: %s/%s \nTemperature: %d \nElapsed Time: %lld "
         "ETA: %lld \nCycle time: %lld Cycle ETA:%lld \nIP: %s",
         ActiveStatus.Cycle, ActiveStatus.Step, ActiveStatus.CurrentTemp,
         ActiveStatus.time_elapsed,

         ActiveStatus.time_full_total, ActiveStatus.time_cycle_total,
         ActiveStatus.time_cycle_total - ActiveStatus.time_elapsed,
         ActiveStatus.IPAddress);
vTaskDelay(pdMS_TO_TICKS(30000));
}
}

void init_status() {
  ActiveStatus.CurrentPower = 0;
  ActiveStatus.CurrentTemp = 0;
  setCharArray(ActiveStatus.Cycle, "Off");
  setCharArray(ActiveStatus.Step, "Off");
  setCharArray(ActiveStatus.IPAddress, "255.255.255.255");

  ActiveStatus.time_full_start = 0;  // time it started
  ActiveStatus.time_full_total = 0;  // expected run time
  ActiveStatus.time_cycle_start = 0; // this cycle start time
  ActiveStatus.time_cycle_total = 0; // maximum run time
  ActiveStatus.time_elapsed = 0;
}

void run_program() {
  for (int i = 0; i < NUM_PROGRAMS; i++) {
    Program_Entry Program = Programs[i];
    if (strcmp(Program.name, ActiveStatus.Program) == 0) {
      _LOG_I("Found Program %s, preparing to run ",Program.name);
      break;
    } else {
      _LOG_I("Program Name: %s", Program.name);
    }

    printf("Program Name: %s\n", Program.name);
  }
};

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  init_status();
  // wifi_init_sta();

  ESP_LOGI("Dishwasher", "Version: %s \nFirmware Update:%s\n", VERSION,
           FIRMWARE_URL);
  printf("\nTotal program count: %d\n", NUM_PROGRAMS);
  for (int i = 0; i < NUM_PROGRAMS; i++) {
    Program_Entry Program = Programs[i];
    printf("Program Name: %s\n", Program.name);
  }

 // run_program("Test");
  setCharArray(ActiveStatus.Program,"Test");
  xTaskCreate(run_program, "Run_Program", 2048, NULL, 5,
              NULL);
              while(1);
}