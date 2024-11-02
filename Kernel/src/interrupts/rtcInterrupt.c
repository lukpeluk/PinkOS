#include <interrupts/rtcInterrupt.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>

void int_28() {
    callRTCHandler(update_time());  
    rtc_acknowledge_interrupt();
}