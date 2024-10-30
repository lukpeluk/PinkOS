#include <interrupts/rtcInterrupt.h>
#include <eventHandling/eventHandlers.h>

void int_28() {
    RTC_Time time = get_time();
    callRTCHandler(time);

    rtc_acknowledge_interrupt();
}