#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <stdint.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t day_of_week;
} RTC_Time;

extern void init_rtc();
extern void rtc_acknowledge_interrupt();

RTC_Time * get_time();

void set_timezone(int time_zone);




#endif