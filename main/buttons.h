#ifndef BUTTONS_H
#define BUTTONS_H

#include "driver/gpio.h"

typedef struct {
  int state;
} button_t;

extern button_t buttons[];
void init_switchesandleds();
void button_monitor_task(void *arg);

#define BUTTON_PRESSED 1
#define BUTTON_RELEASED 0
#define BUTTON_OFF 0

#endif