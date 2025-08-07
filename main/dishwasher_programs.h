#ifndef DISHWASHER_PROGRAM_H
#define DISHWASHER_PROGRAM_H

#include <stdint.h>
#include <driver/gpio.h>        // For GPIO_NUM_X definitions
#include <stddef.h>             // For size_t

#ifndef APP_VERSION
#define APP_VERSION "@VERSION@"
#endif

#ifndef BIT64
#define BIT64(n) (1ULL << (n))
#endif

#define HEAT          (BIT64(GPIO_NUM_32))
#define SPRAY         (BIT64(GPIO_NUM_33))
#define INLET         (BIT64(GPIO_NUM_25))
#define DRAIN         (BIT64(GPIO_NUM_26))
#define SOAP          (BIT64(GPIO_NUM_27))
#define SENSOR_ENABLE (BIT64(GPIO_NUM_18))
#define CLEANLIGHT    (BIT64(GPIO_NUM_19))
#define LIGHT3        (BIT64(GPIO_NUM_21))

#define NUM_PROGRAMS 3

#define SEC           (1ULL)       // 1 second in milliseconds
#define MIN           (60 * SEC)  // 60 seconds in milliseconds

#define NUM_DEVICES   8

static const char *TAG=PROJECT_NAME;
static const char *FIRMWARE_URL="https://house.sjcnu.com/esp32/firmware/" VERSION "/" PROJECT_NAME ".bin";


typedef struct {
    const char *state_name;
    uint32_t    min_time;   // milliseconds
    uint32_t    max_time;   // milliseconds
    int         min_temp;
    int         max_temp;
    uint64_t    gpio_mask;  // BIT64 mask for all pins to set HIGH
} ProgramLineStruct;

typedef struct {
    const char *program_name;
    const ProgramLineStruct *program_lines;
    size_t num_lines;
} Program_Entry;

// Normal program
static const ProgramLineStruct NormalProgramLines[] = {
    {"init",           1,         0,        0,      0,       0                      },

    {"Prep/fill",      3*MIN,     0,        0,      0,       INLET                   },
    {"Prep/Spray",     5*MIN,     0,        0,      0,       SPRAY                   },
    {"Prep/drain",     2*MIN,     0,        0,      0,       DRAIN                   },

    {"wash/fill",      3*MIN,     0,        0,      0,       INLET                   },
    {"wash/preheat",   5*MIN,     40*MIN,   130,    140,     HEAT | SPRAY            },//heat water to at _least_ 130
    {"wash/soap",      1*MIN,     0,        140,    150,     HEAT | SPRAY | SOAP     },
    {"wash/wash",      45*MIN,    75*MIN,   150,    150,     HEAT | SPRAY            },
    {"wash/drain",     2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-1/fill",   3*MIN,     0,        0,      0,       INLET                   },
    {"rinse-1/rinse",  5*MIN,     0,        0,      0,       HEAT | SPRAY            },
    {"rinse-1/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-2/fill",   3*MIN,     0,        0,      0,       INLET                   },
    {"rinse-2/rinse",  5*MIN,     0,        0,      0,       HEAT | SPRAY            },
    {"rinse-2/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-3/fill",   3*MIN,     0,        0,      0,       INLET                   },
    {"rinse-3/soap",   1*MIN,     0,        140,    140,     HEAT | DRAIN | SOAP     }, //rinse aid
    {"rinse-3/rinse",  10*MIN,    20*MIN,   140,    140,     HEAT | SPRAY            },
    {"rinse-3/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"cool/vent",      29*MIN,    0,        0,      0,       HEAT                    },
    {"fini",           0,         0,        0,      0,       0                       }
};
// Test program: all times 30 seconds, temps copied from Normal
static const ProgramLineStruct TestProgramLines[] = {
    {"init",           1,         0,        0,      0,       0                      },

    {"Prep/fill",      30*SEC,    0,        0,      0,       INLET                   },
    {"Prep/Spray",     30*SEC,    30*SEC,   130,    130,     SPRAY                   },
    {"Prep/drain",     2*MIN,     0,        0,      0,       DRAIN                   },

    {"wash/fill",      30*SEC,    0,        0,      0,       INLET                   },
    {"wash/preheat",   0,         30*SEC,   130,    130,     HEAT | SPRAY            },
    {"wash/soap",      30*SEC,    0,        140,    140,     HEAT | SPRAY | SOAP     },
    {"wash/wash",      30*SEC,    30*SEC,   152,    152,     HEAT | SPRAY            },
    {"wash/drain",     2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-1/fill",   30*SEC,    0,        0,      0,       INLET                   },
    {"rinse-1/rinse",  30*SEC,    0,        0,      0,       HEAT | SPRAY            },
    {"rinse-1/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-2/fill",   30*SEC,    0,        0,      0,       INLET                   },
    {"rinse-2/rinse",  30*SEC,    0,        0,      0,       HEAT | SPRAY            },
    {"rinse-2/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-3/fill",   30*SEC,    0,        0,      0,       INLET                   },
    {"rinse-3/soap",   30*SEC,    0,        140,    140,     HEAT | DRAIN | SOAP     },
    {"rinse-3/rinse",  30*SEC,    30*SEC,   140,    140,     HEAT | SPRAY            },
    {"rinse-3/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"cool/vent",      29*MIN,    0,        0,      0,       HEAT                    },
    {"fini",           0,         0,        0,      0,       0                       }
};

// Hi-Temp program: same times as Normal, but all wash temps set to 160
static const ProgramLineStruct HiTempProgramLines[] = {
    {"init",           1,         0,        0,      0,       0                      },

    {"Prep/fill",      3*MIN,     0,        0,      0,       INLET                   },
    {"Prep/Spray",     5*MIN,     0,        0,      0,       SPRAY                   },
    {"Prep/drain",     2*MIN,     0,        0,      0,       DRAIN                   },

    {"wash/fill",      3*MIN,     0,        0,      0,       INLET                   },
    {"wash/preheat",   0,         40*MIN,   160,    160,     HEAT | SPRAY            },
    {"wash/soap",      1*MIN,     0,        160,    160,     HEAT | SPRAY | SOAP     },
    {"wash/wash",      45*MIN,    75*MIN,   160,    160,     HEAT | SPRAY            },
    {"wash/drain",     2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-1/fill",   3*MIN,     0,        0,      0,       INLET                   },
    {"rinse-1/rinse",  5*MIN,     0,        0,      0,       HEAT | SPRAY            },
    {"rinse-1/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-2/fill",   3*MIN,     0,        0,      0,       INLET                   },
    {"rinse-2/rinse",  5*MIN,     0,        0,      0,       HEAT | SPRAY            },
    {"rinse-2/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"rinse-3/fill",   3*MIN,     0,        0,      0,       INLET                   },
    {"rinse-3/soap",   1*MIN,     0,        160,    160,     HEAT | DRAIN | SOAP     },
    {"rinse-3/rinse",  10*MIN,    20*MIN,   160,    160,     HEAT | SPRAY            },
    {"rinse-3/drain",  2*MIN,     0,        0,      0,       DRAIN                   },

    {"cool/vent",      29*MIN,    0,        140,      140,       HEAT                    },
    {"fini",           0,         0,        0,      0,       0                       }
};

static const Program_Entry Programs[NUM_PROGRAMS] = {
    {"Normal", NormalProgramLines, sizeof(NormalProgramLines) / sizeof(NormalProgramLines[0])},
    {"Test",   TestProgramLines,   sizeof(TestProgramLines) / sizeof(TestProgramLines[0])},
    {"Hi-Temp",HiTempProgramLines, sizeof(HiTempProgramLines) / sizeof(HiTempProgramLines[0])}
};
#endif