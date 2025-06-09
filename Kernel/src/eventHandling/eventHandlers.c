#include <eventHandling/eventHandlers.h>
#include <eventHandling/handlerIds.h>
#include <processManager/processState.h>

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
    RestoreContextHandler restore_context_handler;
    UserEnvironmentApiHandler user_environment_api_handler;
    ExceptionHandler exception_handler;
    RegistersHandler registers_handler;
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
    case RESTORE_CONTEXT_HANDLER:
        eventHandlers.restore_context_handler = (RestoreContextHandler)handler;
        break;
    case USER_ENVIRONMENT_API_HANDLER:
        eventHandlers.user_environment_api_handler = (UserEnvironmentApiHandler)handler;
        break;
    case EXCEPTION_HANDLER:
        eventHandlers.exception_handler = (ExceptionHandler)handler;
        break;
    case REGISTERS_HANDLER:
        eventHandlers.registers_handler = (RegistersHandler)handler;
        break;
    
    default:
        break;
    }
}


// functions to call each handler (wrapping the actual handler and testing for null first to avoid segfaults)
// Also, it activates root mode before calling the handler

void callKeyHandler(char event_type, int hold_times, char ascii, char scan_code) {
    CALL_IF_IMPLEMENTED(key_handler, event_type, hold_times, ascii, scan_code);
}

void callTickHandler(unsigned long ticks) {
    CALL_IF_IMPLEMENTED(tick_handler, ticks);
}

void callRTCHandler(RTC_Time * time) {
    CALL_IF_IMPLEMENTED(rtc_handler, time);
}

void callRestoreContextHandler(uint8_t was_graphic) {
    CALL_IF_IMPLEMENTED(restore_context_handler, was_graphic);
}

void callUserEnvironmentApiHandler(uint64_t endpoint_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    CALL_IF_IMPLEMENTED(user_environment_api_handler, endpoint_id, arg1, arg2, arg3, arg4);
}

void callExceptionHandler(int exception_id, BackupRegisters * backup_registers) {
    CALL_IF_IMPLEMENTED(exception_handler, exception_id, backup_registers);
}

void callRegistersHandler(BackupRegisters * backup_registers) {
    CALL_IF_IMPLEMENTED(registers_handler, backup_registers);
}

