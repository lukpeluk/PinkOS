#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <stdint.h>

void initEventManager();
void registerEventSubscription(int eventId, Pid pid, void (*handler)(void* data));
void registerEventWaiting(int eventId, Pid pid, void* data);
void unregisterEventSubscription(int eventId, Pid pid);

void handleProcessDeath(Pid pid);
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