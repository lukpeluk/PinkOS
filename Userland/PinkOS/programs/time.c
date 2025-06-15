#include <libs/stdpink.h>
#include <stdint.h>

#include <programs.h>
#include <syscalls/syscallCodes.h>
#include <environmentApiEndpoints.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t day_of_week;
} RTC_Time;

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void time_main(char * args){
    RTC_Time time;
    syscall(GET_RTC_TIME_SYSCALL, (uint64_t) &time, 0, 0, 0, 0);
    printf((char *)"Son las %d:%d:%d\n", (int)time.hours, (int)time.minutes, (int)time.seconds);
}