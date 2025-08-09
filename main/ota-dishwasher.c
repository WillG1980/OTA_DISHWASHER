// main.c
#include "driver/gpio.h"
#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#include "analog.h"
#include "buttons.h"
#include "dishwasher_programs.h"
#include "local_ota.h"   // <- use headers, not .c
#include "local_time.h"
#include "local_wifi.h"  // <- use headers, not .c

#define __TAG__ "OTA_DISHWASHER"

// global status
status_struct ActiveStatus;

// prototypes (task functions must be of type void f(void *))
static void monitor_task_buttons(void *pvParameters);
static void monitor_task_temperature(void *pvParameters);
static void update_published_status(void *pvParameters);
static void run_program(void *pvParameters);
static void _init_setup(void);
static void init_status();
// ----- implementations -----

static void _init_setup(void)
{
    // initialize subsystems (these functions should be provided by their modules)
    wifi_init_sta();
    init_switchesandleds();
//init_adc();
    init_status();

    // create background monitoring tasks (use reasonable stack sizes)
    xTaskCreate(monitor_task_buttons, "monitor_task_buttons", 4096, NULL, 5, NULL);
    xTaskCreate(monitor_task_temperature, "monitor_task_temperature", 4096, NULL, 5, NULL);
    xTaskCreate(update_published_status, "update_published_status", 4096, NULL, 5, NULL);

    // wait (up to 60s) for wifi
    for (int t = 60; t > 0; t--) {
        _LOG_I("Waiting for wifi, %d seconds remaining", t);
        if (is_connected()) {
            _LOG_I("Connected to Wifi");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void monitor_task_buttons(void *pvParameters)
{
    (void)pvParameters;
    // TODO: implement real button monitoring
    while (1) {
        // poll or wait on interrupts/queue
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void monitor_task_temperature(void *pvParameters)
{
    (void)pvParameters;
    // TODO: implement real temperature monitoring
    while (1) {
        // sample ADC/thermistor
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void update_published_status(void *pvParameters)
{
    (void)pvParameters;
    while (1) {
        printf("Status update: State: %s/%s \nTemperature: %d \nElapsed Time: %lld "
               "Full ETA: %lld \nCycle time: %lld Cycle ETA: %lld \nIP: %s\n",
               ActiveStatus.Cycle, ActiveStatus.Step, ActiveStatus.CurrentTemp,
               (long long)ActiveStatus.time_elapsed,
               (long long)ActiveStatus.time_full_total,
               (long long)ActiveStatus.time_cycle_total,
               (long long)(ActiveStatus.time_cycle_total - ActiveStatus.time_elapsed),
               ActiveStatus.IPAddress);

        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

// run_program task: summarises and then blocks
static void run_program(void *pvParameters)
{
    (void)pvParameters;

    // find chosen program
    Program_Entry chosen = {0};
    bool found = false;
    for (int i = 0; i < NUM_PROGRAMS; i++) {
        if (strcmp(Programs[i].name, ActiveStatus.Program) == 0) {
            chosen = Programs[i];
            found = true;
            _LOG_I("Found Program %s, preparing to run", chosen.name);
            break;
        }
    }
    if (!found) {
        _LOG_W("Program '%s' not found. Exiting run_program task.", ActiveStatus.Program);
        vTaskDelete(NULL);
        return;
    }

    // compute min/max times and print steps
    long long min_time = 0;
    long long max_time = 0;
    const char *old_cycle = ""; // safe empty string

    for (size_t l = 0; l < chosen.num_lines; l++) {
        const ProgramLineStruct *Line = &chosen.lines[l];

        if (strcmp(old_cycle, Line->name_cycle) != 0) {
            printf("\n-- new cycle: %s --\n", Line->name_cycle);
        }
        old_cycle = Line->name_cycle;

        min_time += (long long)Line->min_time;
        // if max_time = 0 treat as min_time (as your design did)
        max_time += (long long)((Line->max_time > 0) ? Line->max_time : Line->min_time);

        printf("%02zu: Cycle: %s / %s; Time [ Min %lu Max %lu ]  GPIO mask: 0x%llx\n",
               l, Line->name_cycle, Line->name_step,
               Line->min_time, Line->max_time, (unsigned long long)Line->gpio_mask);
        printf("\tRunning totals: Min: %lld, Max: %lld\n", (long long)min_time, (long long)max_time);
    }

    printf("\nTotal run time for program '%s': Min: %lld ms, Max: %lld ms\n", chosen.name, (long long)min_time, (long long)max_time);

    // TODO: implement actual runtime control of GPIOs, temps, timing, etc.
    // For now block forever (or you could vTaskDelete(NULL) to end the task)
        printf("\nIn final closeout - power cycle to restart/open door\n");
    
    // never reached
     vTaskDelete(NULL);
}

void init_status(void)
{
    ActiveStatus.CurrentPower = 0;
    ActiveStatus.CurrentTemp = 0;
    setCharArray(ActiveStatus.Cycle, "Off");
    setCharArray(ActiveStatus.Step, "Off");
    setCharArray(ActiveStatus.IPAddress, "255.255.255.255");

    ActiveStatus.time_full_start = 0;
    ActiveStatus.time_full_total = 0;
    ActiveStatus.time_cycle_start = 0;
    ActiveStatus.time_cycle_total = 0;
    ActiveStatus.time_elapsed = 0;
}

// app_main
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    init_status();
_init_setup();
    ESP_LOGI(__TAG__, "Version: %s \nFirmware Update: %s\n", VERSION, FIRMWARE_URL);

    printf("\nTotal program count: %d\n", NUM_PROGRAMS);
    for (int i = 0; i < NUM_PROGRAMS; i++) {
        printf("Program Name: %s\n", Programs[i].name);
    }

    // start wifi, monitors, etc
    

    // choose program and start program task
    setCharArray(ActiveStatus.Program, "Test");
    xTaskCreate(run_program, "Run_Program", 8192, NULL, 5, NULL);

    // Keep main alive but yield CPU — do not busy-loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
