#include <interrupts/rtcInterrupt.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <eventManager/eventManager.h>

void int_28() {
    // callRTCHandler(update_time());  
    RTC_Time time = * update_time();
    rtc_acknowledge_interrupt();
    handleRTCEvent(time);
}