#include <eventManager/eventManager.h>
#include <eventManager/eventIds.h>

#include <memoryManager/memoryManager.h>
#include <drivers/serialDriver.h>
#include <lib.h>


#define MAX_EVENTS 6

#define NULL ((void*)0)

typedef enum {
    SUSCRIPTION,
    WAITING,
} EventType;
typedef struct Listener {
    Pid pid;
    void (*handler)(void* data);            // Para las suscripciones, es el handler que se va a llamar cuando ocurra el evento, como argumento se le pasa un puntero a los datos del evento
    void* data;                             // Para los waiting, es el dato que se memcopiará al puntero del evento
    EventType type;
    // char * arguments; // Arguments for the handler, if needed
    struct Listener* next; // Pointer to the next listener in the linked list
} Listener;

typedef struct {
    int id;
    uint64_t size;
    Listener* listeners; // Pointer to the first listener in a linked list

} Event;


typedef struct EventManager {
    Event events[MAX_EVENTS];
} EventManager;

static EventManager eventManager;

void initEventManager() {
//     log_to_serial("Event Manager initialized");
    eventManager.events[KEY_EVENT].id = KEY_EVENT;
    eventManager.events[KEY_EVENT].size = sizeof(KeyboardEvent);
    eventManager.events[KEY_EVENT].listeners = NULL;
//     log_to_serial("Event Manager initialized: KEY_EVENT");
//     log_decimal("Event size: ", eventManager.events[KEY_EVENT].size);



    eventManager.events[SLEEP_EVENT].id = SLEEP_EVENT;
    eventManager.events[SLEEP_EVENT].size = sizeof(uint64_t);
    eventManager.events[SLEEP_EVENT].listeners = NULL;
//     log_to_serial("Event Manager initialized: SLEEP_EVENT");
//     log_decimal("Event size: ", eventManager.events[SLEEP_EVENT].size);

    eventManager.events[RTC_EVENT].id = RTC_EVENT;
    eventManager.events[RTC_EVENT].size = sizeof(RTC_Time);
    eventManager.events[RTC_EVENT].listeners = NULL;
//     log_to_serial("Event Manager initialized: RTC_EVENT");
//     log_decimal("Event size: ", eventManager.events[RTC_EVENT].size);

    eventManager.events[PROCESS_DEATH_EVENT].id = PROCESS_DEATH_EVENT;
    eventManager.events[PROCESS_DEATH_EVENT].size = sizeof(Pid);
    eventManager.events[PROCESS_DEATH_EVENT].listeners = NULL;
//     log_to_serial("Event Manager initialized: PROCESS_DEATH_EVENT");
//     log_decimal("Event size: ", eventManager.events[PROCESS_DEATH_EVENT].size);

    eventManager.events[EXCEPTION_EVENT].id = EXCEPTION_EVENT;
    eventManager.events[EXCEPTION_EVENT].size = sizeof(Exception);
    eventManager.events[EXCEPTION_EVENT].listeners = NULL;
//     log_to_serial("Event Manager initialized: EXCEPTION_EVENT");
//     log_decimal("Event size: ", eventManager.events[EXCEPTION_EVENT].size);

    eventManager.events[BROADCAST_EVENT].id = BROADCAST_EVENT;
    eventManager.events[BROADCAST_EVENT].size = 0; // Broadcast events don't have a specific size
    eventManager.events[BROADCAST_EVENT].listeners = NULL;
//     log_to_serial("Event Manager initialized: BROADCAST_EVENT");
//     log_decimal("Event size: ", eventManager.events[BROADCAST_EVENT].size);
    
}

void registerEventSubscription(int eventId, Pid pid, void (*handler)(void* data)) {
    if (eventId < 0 || eventId >= MAX_EVENTS) {
        return; // Invalid event ID
    }

    Listener* newListener = (Listener*) malloc(sizeof(Listener));
    if (!newListener) {
        return; // Memory allocation failed
    }

    newListener->pid = pid;
    newListener->handler = handler;
    newListener->type = SUSCRIPTION;

    // Add the new listener to the linked list of listeners for the event
    Listener* current = eventManager.events[eventId].listeners;
    newListener->next = current;
    eventManager.events[eventId].listeners = newListener;       // Para que sea O(1) en vez de O(n) al agregar un listener (el señorito se quejaba de que era O(n))

}

void registerEventWaiting(int eventId, Pid pid, void* data) {
    if (eventId < 0 || eventId >= MAX_EVENTS) {
        return; // Invalid event ID
    }

    Listener* newListener = (Listener*) malloc(sizeof(Listener));
    if (!newListener) {
        return; // Memory allocation failed
    }

    newListener->pid = pid;
    newListener->data = data;
    newListener->type = WAITING;

    // Add the new listener to the linked list of listeners for the event
    Listener* current = eventManager.events[eventId].listeners;
    newListener->next = current;
    eventManager.events[eventId].listeners = newListener;       // Para que sea O(1) en vez de O(n) al agregar un listener (el señorito se quejaba de que era O(n))
    setWaiting(pid); // Set the process as waiting, so it can be woken up later when the event occurs
}

void unregisterEventSubscription(int eventId, Pid pid) {
    if (eventId < 0 || eventId >= MAX_EVENTS) {
        return; // Invalid event ID
    }

    Listener* current = eventManager.events[eventId].listeners;
    Listener* previous = NULL;

    while (current != NULL) {
        if (current->pid == pid && current->type == SUSCRIPTION) {
            // Found the listener to remove
            if (previous == NULL) {
                // Removing the first listener in the list
                eventManager.events[eventId].listeners = current->next;
            } else {
                // Removing a listener in the middle or end of the list
                previous->next = current->next;
            }
            free(current); // Free the memory allocated for the listener
            return; // Exit after removing the listener
        }
        previous = current;
        current = current->next;
    }
}


void notifyEvent(int eventId, void* data) {
    if (eventId < 0 || eventId >= MAX_EVENTS) {
        return; // Invalid event ID
    }

    Listener* current = eventManager.events[eventId].listeners;
    while (current != NULL) {
        if (current->type == SUSCRIPTION) {

            void *eventData = malloc(eventManager.events[eventId].size);
            if (!eventData) {
                // Handle error: memory allocation failed
                // You might want to log this or take some other action
                return;
            }  
            // Copy the data to the eventData buffer
            memcpy(eventData, data, eventManager.events[eventId].size);
            
            // Create a thread to handle the event
            Pid pid = newThread(current->handler, eventData, PRIORITY_NORMAL, current->pid);
            if (pid == 0) {
                // Handle error: could not create thread
                // You might want to log this or take some other action
            } else {
                // Successfully created thread to handle the event
                // You can also store the PID of the thread if needed
            }
        } else if (current->type == WAITING) {
            // Notify to the scheduler that the process is not waiting anymore
            memcpy(current->data, data, eventManager.events[eventId].size);
            wakeProcess(current->pid);
            
        }
        current = current->next;
    }
}

void handleProcessDeath(Pid pid) {
    // Remove all the listeners with the given PID from all events
    for (int i = 0; i < MAX_EVENTS; i++) {
        Listener* current = eventManager.events[i].listeners;
        Listener* previous = NULL;  
        while (current != NULL) {
            if (current->pid == pid) {
                // Found the listener to remove
                if (previous == NULL) {
                    // Removing the first listener in the list
                    eventManager.events[i].listeners = current->next;
                } else {
                    // Removing a listener in the middle or end of the list
                    previous->next = current->next;
                }
                free(current); // Free the memory allocated for the listener
                current = (previous == NULL) ? eventManager.events[i].listeners : previous->next; // Move to next listener
            } else {
                previous = current;
                current = current->next;
            }
        }
    }

    notifyEvent(PROCESS_DEATH_EVENT, &pid);
}

void handleSleep(uint64_t millis) {
    notifyEvent(SLEEP_EVENT, &millis);
}

void handleRTCEvent(RTC_Time time) {
    notifyEvent(RTC_EVENT, &time);
}

void handleKeyEvent(KeyboardEvent keyEvent) {
    notifyEvent(KEY_EVENT, &keyEvent);
}

void handleException(Exception exception) {
    notifyEvent(EXCEPTION_EVENT, &exception);
}

