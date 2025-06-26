#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <stdint.h>
#include <types.h>

// Drivers and Modules
#include <drivers/keyboardDriver.h>
#include <drivers/pitDriver.h>
#include <drivers/rtcDriver.h>
#include <exceptions/exceptions.h>
#include <processManager/scheduler.h>

typedef struct KeyboardCondition {
    char scan_code; // The scan code of the key to filter
    char ascii;     // The ASCII character to filter
} KeyboardCondition;

typedef struct RTCCondition {
    uint8_t seconds; // Seconds to filter
    uint8_t minutes; // Minutes to filter
    uint8_t hours;   // Hours to filter
} RTCCondition;

typedef struct SleepCondition {
    uint64_t millis; // Milliseconds to filter
} SleepCondition;

typedef struct ProcessDeathCondition {
    Pid pid; // PID of the process that died
} ProcessDeathCondition;

typedef struct ProcessDetachingCondition {
    Pid pid; // PID of the process that is being detached
} ProcessDetachingCondition;

typedef struct ExceptionCondition {
    uint64_t exception_id; // Exception ID to filter
} ExceptionCondition;



void initEventManager();
void registerEventSubscription(int eventId, Pid pid, void (*handler)(void* data), void* condition_data);
void registerEventWaiting(int eventId, Pid pid, void* data, void* condition_data);
void unregisterEventSubscription(int eventId, Pid pid);

void handleProcessDeath(Pid pid);
void handleProcessDetaching(Pid pid);
void handleSleep(uint64_t millis);
void handleRTCEvent(RTC_Time time);
void handleKeyEvent(KeyboardEvent keyEvent);
void handleException(Exception exception);




// Muerte de un proceso
// Sleep
// Hora RTC -> cada segundo o cada x tiempo (cronjob :glasses:)
// Teclado
// Excepción
// Broadcast



#endif