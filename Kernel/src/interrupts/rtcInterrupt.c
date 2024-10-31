#include <interrupts/rtcInterrupt.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>

void int_28() {
    RTC_Time time = get_time();
    callRTCHandler(time);
    char timeString[9];
    timeString[0] = '0' + time.hours / 10;
    timeString[1] = '0' + time.hours % 10;
    timeString[2] = ' ';
    timeString[3] = '0' + time.minutes / 10;
    timeString[4] = '0' + time.minutes % 10;
    timeString[5] = ' ';
    timeString[6] = '0' + time.seconds / 10;
    timeString[7] = '0' + time.seconds % 10;
    timeString[8] = '\0';

    drawStringAt(timeString, 0x00FFFFFF, 0x00000000, (Point){0, 0});  

    rtc_acknowledge_interrupt();
}