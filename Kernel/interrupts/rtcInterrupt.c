#include <rtcInterrupt.h>
#include <eventHandlers.h>

void int_28() {
    RTC_Time time = get_time();
    if (eventHandlers.rtc_handler != 0)
    {
        eventHandlers.rtc_handler(time);
    }

    rtc_acknowledge_interrupt();
}