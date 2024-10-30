#include <eventHandling/eventHandlers.h>
#include <eventHandling/handlerIds.h>
#include <kernelState.h>

#define NOT_SET 0
// calls the handler if it is implemented setting the kernel in root mode first
#define CALL_IF_IMPLEMENTED(handler, ...) if(eventHandlers.handler != NOT_SET)      \
                                            {                                       \
                                                activateRootMode();                 \
                                                eventHandlers.handler(__VA_ARGS__); \
                                                desactivateRootMode();              \
                                            }                   

// TODO: this should be renamed to eventHandlerManager
// should expose functions to register handlers and call them
// the struct should be private

// stores pointers to the handler functions
typedef struct EventHandlers {
    KeyHandler key_handler; 
    TickHandler tick_handler; 
    RTCHandler rtc_handler;
} EventHandlers;


static EventHandlers eventHandlers = {NOT_SET}; // Initialize all event handlers to null

// registers a handler for a given event, based on the handler_id and the handler function pointer
void registerHandler(uint32_t handler_id, void * handler) {
    switch (handler_id)
    {
    case KEY_HANDLER:
        eventHandlers.key_handler = (KeyHandler)handler;
        break;
    case TICK_HANDLER:
        eventHandlers.tick_handler = (TickHandler)handler;
        break;
    case RTC_HANDLER:
        eventHandlers.rtc_handler = (RTCHandler)handler;
        break;
    
    default:
        break;
    }
}


// functions to call each handler (wrapping the actual handler and testing for null first to avoid segfaults)
// Also, it activates root mode before calling the handler

void callKeyHandler(char key){
    CALL_IF_IMPLEMENTED(key_handler, key);
}

void callTickHandler(unsigned long ticks) {
    CALL_IF_IMPLEMENTED(tick_handler, ticks);
}

void callRTCHandler(RTC_Time time) {
    CALL_IF_IMPLEMENTED(rtc_handler, time);
}


