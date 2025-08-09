#ifndef DISHWASHER_PROGRAM_H
#define DISHWASHER_PROGRAM_H

#include <driver/gpio.h> // For GPIO_NUM_X definitions
#include <stddef.h>      // For size_t
#include <stdint.h>

#define _LOG_I(fmt, ...)                                                       \
  ESP_LOGI(__TAG__, "[%s-%s:%d] " fmt, __func__, __VERSION__, __LINE__,        \
           ##__VA_ARGS__)
#define _LOG_W(fmt, ...)                                                       \
  ESP_LOGW(__TAG__, "[%s-%s:%d] " fmt, __func__, __VERSION__, __LINE__,        \
           ##__VA_ARGS__)
#define _LOG_E(fmt, ...)                                                       \
  ESP_LOGE(__TAG__, "[%s-%s:%d] " fmt, __func__, __VERSION__, __LINE__,        \
           ##__VA_ARGS__)
#define _LOG_D(fmt, ...)                                                       \
  ESP_LOGD(__TAG__, "[%s-%s:%d] " fmt, __func__, __VERSION__, __LINE__,        \
           ##__VA_ARGS__)

#ifndef APP_VERSION
#define APP_VERSION VERSION
#endif

#ifndef BIT64
#define BIT64(n) (1ULL << (n))
#endif



#define HEAT (BIT64(GPIO_NUM_32))
#define SPRAY (BIT64(GPIO_NUM_33))
#define INLET (BIT64(GPIO_NUM_25))
#define DRAIN (BIT64(GPIO_NUM_26))
#define SOAP (BIT64(GPIO_NUM_27))
#define SENSOR_ENABLE (BIT64(GPIO_NUM_18))
#define CLEANLIGHT (BIT64(GPIO_NUM_19))
#define LIGHT3 (BIT64(GPIO_NUM_21))

#define NUM_PROGRAMS 3

#define SEC (1ULL)     // 1 second in milliseconds
#define MIN (60 * SEC) // 60 seconds in milliseconds

#define NUM_DEVICES 8

static const char *FIRMWARE_URL = "https://house.sjcnu.com/esp32/firmware/" OTA_VERSION "/" PROJECT_NAME ".bin";

typedef struct {
  const char *name_cycle;
  const char *name_step;
  uint32_t min_time; // milliseconds
  uint32_t max_time; // milliseconds
  int min_temp;
  int max_temp;
  uint64_t gpio_mask; // BIT64 mask for all pins to set HIGH
} ProgramLineStruct;

typedef struct {
  const char *name;
  const ProgramLineStruct *lines;
  size_t num_lines;
} Program_Entry;

// Normal program
static const ProgramLineStruct NormalProgramLines[] = {
    {"init", "setup", 1, 0, 0, 0, 0},

    {"Prep", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"Prep", "Spray", 5 * MIN, 0, 0, 0, SPRAY},
    {"Prep", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"wash", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"wash", "preheat", 5 * MIN, 40 * MIN, 130, 140,
     HEAT | SPRAY}, // heat water to at _least_ 130
    {"wash", "soap", 1 * MIN, 0, 140, 150, HEAT | SPRAY | SOAP},
    {"wash", "wash", 45 * MIN, 75 * MIN, 150, 150, HEAT | SPRAY},
    {"wash", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse1", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"rinse1", "rinse", 5 * MIN, 0, 0, 0, HEAT | SPRAY},
    {"rinse1", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse2", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"rinse2", "rinse", 5 * MIN, 0, 0, 0, HEAT | SPRAY},
    {"rinse2", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse3", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"rinse3", "soap", 1 * MIN, 0, 140, 140, HEAT | DRAIN | SOAP}, // rinse aid
    {"rinse3", "rinse", 10 * MIN, 20 * MIN, 140, 140, HEAT | SPRAY},
    {"rinse3", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"cool", "vent", 29 * MIN, 0, 0, 0, HEAT},
    {"fini", "clean", 0, 0, 0, 0, 0}};
// Test program: all times 30 seconds, temps copied from Normal
static const ProgramLineStruct TestProgramLines[] = {
    {"init", "setup", 1, 0, 0, 0, 0},

    {"Prep", "fill", 30 * SEC, 0, 0, 0, INLET},
    {"Prep", "Spray", 30 * SEC, 30 * SEC, 130, 130, SPRAY},
    {"Prep", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"wash", "fill", 30 * SEC, 0, 0, 0, INLET},
    {"wash", "preheat", 0, 30 * SEC, 130, 130, HEAT | SPRAY},
    {"wash", "soap", 30 * SEC, 0, 140, 140, HEAT | SPRAY | SOAP},
    {"wash", "wash", 30 * SEC, 30 * SEC, 152, 152, HEAT | SPRAY},
    {"wash", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse1", "fill", 30 * SEC, 0, 0, 0, INLET},
    {"rinse1", "rinse", 30 * SEC, 0, 0, 0, HEAT | SPRAY},
    {"rinse1", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse2", "fill", 30 * SEC, 0, 0, 0, INLET},
    {"rinse2", "rinse", 30 * SEC, 0, 0, 0, HEAT | SPRAY},
    {"rinse2", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse3", "fill", 30 * SEC, 0, 0, 0, INLET},
    {"rinse3", "soap", 30 * SEC, 0, 140, 140, HEAT | DRAIN | SOAP},
    {"rinse3", "rinse", 30 * SEC, 30 * SEC, 140, 140, HEAT | SPRAY},
    {"rinse3", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"cool", "vent", 29 * MIN, 0, 0, 0, HEAT},
    {"fini", "clean", 0, 0, 0, 0, 0}

};
// Hi-Temp program: same times as Normal, but all wash temps set to 160
static const ProgramLineStruct HiTempProgramLines[] = {
    {"init", "setup", 1, 0, 0, 0, 0},

    {"Prep", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"Prep", "Spray", 5 * MIN, 0, 0, 0, SPRAY},
    {"Prep", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"wash", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"wash", "preheat", 0, 40 * MIN, 160, 160, HEAT | SPRAY},
    {"wash", "soap", 1 * MIN, 0, 160, 160, HEAT | SPRAY | SOAP},
    {"wash", "wash", 45 * MIN, 75 * MIN, 160, 160, HEAT | SPRAY},
    {"wash", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse1", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"rinse1", "rinse", 5 * MIN, 0, 0, 0, HEAT | SPRAY},
    {"rinse1", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse2", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"rinse2", "rinse", 5 * MIN, 0, 0, 0, HEAT | SPRAY},
    {"rinse2", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"rinse3", "fill", 3 * MIN, 0, 0, 0, INLET},
    {"rinse3", "soap", 1 * MIN, 0, 160, 160, HEAT | DRAIN | SOAP},
    {"rinse3", "rinse", 10 * MIN, 20 * MIN, 160, 160, HEAT | SPRAY},
    {"rinse3", "drain", 2 * MIN, 0, 0, 0, DRAIN},

    {"cool", "vent", 29 * MIN, 0, 140, 140, HEAT},
    {"fini", "clean", 0, 0, 0, 0, 0}};

static const Program_Entry Programs[NUM_PROGRAMS] = {
    {"Test", TestProgramLines,
     sizeof(TestProgramLines) / sizeof(TestProgramLines[0])},
    {"Normal", NormalProgramLines,
     sizeof(NormalProgramLines) / sizeof(NormalProgramLines[0])},
    {"Hi-Temp", HiTempProgramLines,
     sizeof(HiTempProgramLines) / sizeof(HiTempProgramLines[0])}};

#define setCharArray(target, value)                                            \
  do {                                                                         \
    strncpy((target), (value), sizeof(target) - 1);                            \
    (target)[sizeof(target) - 1] = '\0';                                       \
  } while (0)

typedef struct {
  int CurrentTemp;
  int CurrentPower;
  int64_t time_full_start;
  int64_t time_full_total;

  int64_t time_cycle_start;
  int64_t time_cycle_total;

  int64_t time_total;
  int64_t time_elapsed;
  int64_t time_start;
  char Cycle[10];
  char Step[10];
  char statusstring[512]; // OPTIMIZATION: Fixed size buffer
  char IPAddress[16];     // OPTIMIZATION: Fixed size for IP
  char Program[10];
  bool HEAT_REQUESTED;
} status_struct;
#endif