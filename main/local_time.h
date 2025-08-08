#ifndef TIME_UTILS_H
#define TIME_UTILS_H
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

time_t get_unix_epoch(void);
void print_us_time(void);

#ifdef __cplusplus
}
#endif

#endif // TIME_UTILS_H
