#include <interrupts/rtcInterrupt.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>

static RTC_Time * time;

void int_28() {
    time = get_time();
    callRTCHandler(time);  
    rtc_acknowledge_interrupt();
}