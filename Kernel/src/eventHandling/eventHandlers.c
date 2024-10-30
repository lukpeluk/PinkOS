#include <eventHandling/eventHandlers.h>

#define NOT_SET 0
#define CALL_IF_IMPLEMENTED(handler, ...) if(eventHandlers.handler != NOT_SET) eventHandlers.handler(__VA_ARGS__)

// TODO: this should be renamed to eventHandlerManager
// should expose functions to register handlers and call them
// the struct should be private

// stores pointers to the handler functions
typedef struct EventHandlers {
    KeyHandler key_handler; 
    TickHandler tick_handler; 
    RTCHandler rtc_handler;
} EventHandlers;


static EventHandlers eventHandlers = {0}; // Initialize all event handlers to null

// registers a handler for a given event, based on the handler_id and the handler function pointer
void registerHandler(uint32_t handler_id, void * handler) {
    // TODO: define handler_id constants
    switch (handler_id)
    {
    case 0:
        eventHandlers.key_handler = (KeyHandler)handler;
        break;
    case 1:
        eventHandlers.tick_handler = (TickHandler)handler;
        break;
    case 2:
        eventHandlers.rtc_handler = (RTCHandler)handler;
        break;
    
    default:
        break;
    }
}


// functions to call each handler (wrapping the actual handler and testing for null first)

void callKeyHandler(char key){
    CALL_IF_IMPLEMENTED(key_handler, key);
}

void callTickHandler(unsigned long ticks) {
    CALL_IF_IMPLEMENTED(tick_handler, ticks);
}

void callRTCHandler(RTC_Time time) {
    CALL_IF_IMPLEMENTED(rtc_handler, time);
}


