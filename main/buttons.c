#include "buttons.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_GPIO GPIO_NUM_19
#define BUTTON_GPIO GPIO_NUM_16
#define GND_GPIO GPIO_NUM_17
extern const char *__TAG__;

button_t buttons[1];

void init_switchesandleds() {
  gpio_config_t sw_conf = {
    .pin_bit_mask = (1ULL << BUTTON_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&sw_conf);

  gpio_config_t led_conf = {
    .pin_bit_mask = (1ULL << LED_GPIO),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&led_conf);

  gpio_config_t gnd_conf = {
    .pin_bit_mask = (1ULL << GND_GPIO),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&gnd_conf);

  gpio_set_level(GND_GPIO, 0); // Provide GND ref
}

void button_monitor_task(void *arg) {
  const TickType_t debounce_ticks = pdMS_TO_TICKS(50);
  int last_state = 1;
  TickType_t last_change_time = 0;

  while (true) {
    int current_state = gpio_get_level(BUTTON_GPIO);
    TickType_t current_time = xTaskGetTickCount();

    if (current_state != last_state) {
      if (current_time - last_change_time > debounce_ticks) {
        if (current_state == 0) {
          buttons[0].state = BUTTON_PRESSED;
          gpio_set_level(LED_GPIO, 1);
        } else {
          buttons[0].state = BUTTON_RELEASED;
          gpio_set_level(LED_GPIO, 0);
        }
        last_change_time = current_time;
      }
      last_state = current_state;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
