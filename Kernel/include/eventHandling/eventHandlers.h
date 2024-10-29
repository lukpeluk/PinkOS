#ifndef _EVENT_HANDLERS_H
#define _EVENT_HANDLERS_H

#include <drivers/rtcDriver.h>

// The kernel allows processes to register handlers for different events
// To be able to register a handler, the process must have the appropriate permissions
// That allows the OS itself to register handlers but ensures no other process can overwrite them 
// (for instance, the snake game can't register handlers, but the shell can)
// This event system intends to be a notification framework, a bit like inotify in Linux, trying to overcome the limitations of a single-process OS
// This allows the user environment (kind of analogous concept to GNOME in Ubuntu) to do stuff like handling key presses, ticks, etc... without polling and without being in the foreground
// An example of this is being able to quit processes when esc is pressed, even if the shell is not running
// Also it can update the status bar and other OS related things    

// The handlers are pointers to functions, and are registered via a syscall
// The kernel keeps track of them in this file, in the EventHandlers struct
// When an event ocurrs, it is it's responsibility to call the appropriate handler

// types for the differeng handler functions 
typedef void (*KeyHandler)(char key); // Key press handler, TODO: add key press/release flag and scancode as args
typedef void (*TickHandler)(unsigned long ticks); // Tick handler, to be called every tick
typedef void (*RTCHandler)(RTC_Time time); // RTC handler, to be called every RTC interrupt

// stores pointers to the handler functions
typedef struct EventHandlers {
    KeyHandler key_handler; 
    TickHandler tick_handler; 
    RTCHandler rtc_handler;
} EventHandlers;

// Extern declaration for the global state variable
extern EventHandlers eventHandlers;

#endif