#ifndef _EVENT_HANDLERS_H
#define _EVENT_HANDLERS_H

#include <drivers/rtcDriver.h>
#include <drivers/registersDriver.h>

// The kernel allows processes to register handlers for different events
// To be able to register a handler, the process must have the appropriate permissions
// That allows the OS itself to register handlers but ensures no other process can overwrite them 
// (for instance, the snake game can't register handlers, but the shell can)
// This event system intends to be a notification framework, a bit like inotify in Linux, trying to overcome the limitations of a single-process OS
// This allows the user environment (kind of analogous concept to GNOME in Ubuntu) to do stuff like handling key presses, ticks, etc... without polling and without being in the foreground
// An example of this is being able to quit processes when esc is pressed, even if the shell is not running
// Also it can update the status bar and other OS related things    

// The handlers are pointers to functions, and are registered via a syscall
// The kernel keeps track of them in eventHandlers.c in the EventHandlers struct
// When an event ocurrs, it is it's responsibility to call the appropriate handler
// To register and call handlers, eventHandlerManager.c is used

// types for the differeng handler functions 
typedef void (*KeyHandler)(char event_type, int hold_times, char ascii, char scan_code); // Key handler, to be called every key press
typedef void (*TickHandler)(unsigned long ticks); // Tick handler, to be called every tick
typedef void (*RTCHandler)(RTC_Time * time); // RTC handler, to be called every RTC interrupt
typedef void (*RestoreContextHandler)(uint8_t was_graphic); // Restore context handler, to be called when the context needs to be restored. Has a flag to indicate if the context was graphic
typedef void (*UserEnvironmentApiHandler)(uint64_t endpoint_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4); // User environment API handler, to be called when a syscall is made by a user environment
typedef void (*ExceptionHandler)(int exception_id, BackupRegisters * backup_registers); // Exception handler, to be called when an exception ocurrs
typedef void (*RegistersHandler)(BackupRegisters * backup_registers); // Registers handler, to be called when an exception ocurrs
// function to register a handler
void registerHandler(uint32_t handler_id, void * handler);

// functions to call a handler (one per handler, wrapping the actual handler and testing for null first)

void callKeyHandler(char event_type, int hold_times, char ascii, char scan_code);
void callTickHandler(unsigned long ticks);
void callRTCHandler(RTC_Time * time);
void callRestoreContextHandler(uint8_t was_graphic);
void callUserEnvironmentApiHandler(uint64_t endpoint_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);
void callExceptionHandler(int exception_id, BackupRegisters * backup_registers);
void callRegistersHandler(BackupRegisters * backup_registers);

#endif