#include <eventManager/eventManager.h>
#include <eventManager/eventIds.h>

#include <memoryManager/memoryManager.h>
#include <windowManager/windowManager.h>
#include <drivers/serialDriver.h>
#include <lib.h>
#include <drivers/pitDriver.h>

#define MAX_EVENTS 6

#define NULL ((void*)0)

typedef enum {
    SUSCRIPTION,
    WAITING,
} EventType;

typedef struct Listener {
    Pid pid;
    void (*handler)(void* data);            // Para las suscripciones, es el handler que se va a llamar cuando ocurra el evento, como argumento se le pasa un puntero a los datos del evento
    void* data;                             // Para los waiting, es el lugar donde se memcopiará la información del evento
    void* condition_data;                   // Esto se le pasa al filter para saber a qué condición se refiere el listener, por ejemplo, si es un evento de teclado, puede ser el código de la tecla que se está esperando  
    EventType type;
    struct Listener* next; // Pointer to the next listener in the linked list
} Listener;

typedef struct {
    int id;
    uint64_t data_size;
    uint64_t condition_data_size; // Size of the condition data, if applicable
    Listener* listeners; // Pointer to the first listener in a linked list
    int one_time; // If true, the event will be removed after being notified once
} Event;


typedef struct EventManager {
    Event events[MAX_EVENTS];
} EventManager;

static EventManager eventManager;
int filterProcessDeathCondition(void* condition_data, void* data);


void initEventManager() {
    // log_to_serial("Event Manager initialized");
    eventManager.events[KEY_EVENT].id = KEY_EVENT;
    eventManager.events[KEY_EVENT].data_size = sizeof(KeyboardEvent);
    eventManager.events[KEY_EVENT].condition_data_size = sizeof(KeyboardCondition); // Size of the condition data for keyboard events
    eventManager.events[KEY_EVENT].listeners = NULL;
    eventManager.events[KEY_EVENT].one_time = 0; // Set to 0 if you want the event to persist after being notified
    // log_to_serial("Event Manager initialized: KEY_EVENT");
    // log_decimal("Event size: ", eventManager.events[KEY_EVENT].data_size);



    eventManager.events[SLEEP_EVENT].id = SLEEP_EVENT;
    eventManager.events[SLEEP_EVENT].data_size = sizeof(uint64_t);
    eventManager.events[SLEEP_EVENT].condition_data_size = sizeof(SleepCondition); // Size of the condition data for sleep events
    eventManager.events[SLEEP_EVENT].listeners = NULL;
    eventManager.events[SLEEP_EVENT].one_time = 1;
    // log_to_serial("Event Manager initialized: SLEEP_EVENT");
    // log_decimal("Event size: ", eventManager.events[SLEEP_EVENT].data_size);

    eventManager.events[RTC_EVENT].id = RTC_EVENT;
    eventManager.events[RTC_EVENT].data_size = sizeof(RTC_Time);
    eventManager.events[RTC_EVENT].condition_data_size = sizeof(RTCCondition); // Size of the condition data for RTC events
    eventManager.events[RTC_EVENT].listeners = NULL;
    eventManager.events[RTC_EVENT].one_time = 0; 
    // log_to_serial("Event Manager initialized: RTC_EVENT");
    // log_decimal("Event size: ", eventManager.events[RTC_EVENT].data_size);

    eventManager.events[PROCESS_DEATH_EVENT].id = PROCESS_DEATH_EVENT;
    eventManager.events[PROCESS_DEATH_EVENT].data_size = sizeof(Pid);
    eventManager.events[PROCESS_DEATH_EVENT].condition_data_size = sizeof(ProcessDeathCondition); // Size of the condition data for process death events
    eventManager.events[PROCESS_DEATH_EVENT].listeners = NULL;
    eventManager.events[PROCESS_DEATH_EVENT].one_time = 1;
    // log_to_serial("Event Manager initialized: PROCESS_DEATH_EVENT");
    // log_decimal("Event size: ", eventManager.events[PROCESS_DEATH_EVENT].data_size);

    eventManager.events[EXCEPTION_EVENT].id = EXCEPTION_EVENT;
    eventManager.events[EXCEPTION_EVENT].data_size = sizeof(Exception);
    eventManager.events[EXCEPTION_EVENT].condition_data_size = sizeof(ExceptionCondition); // Size of the condition data for exception events
    eventManager.events[EXCEPTION_EVENT].listeners = NULL;
    eventManager.events[EXCEPTION_EVENT].one_time = 0;
    // log_to_serial("Event Manager initialized: EXCEPTION_EVENT");
    // log_decimal("Event size: ", eventManager.events[EXCEPTION_EVENT].data_size);

    eventManager.events[BROADCAST_EVENT].id = BROADCAST_EVENT;
    eventManager.events[BROADCAST_EVENT].data_size = 0; // Broadcast events don't have a specific size
    eventManager.events[BROADCAST_EVENT].listeners = NULL;
    eventManager.events[BROADCAST_EVENT].condition_data_size = 0; // No condition data for broadcast events
    eventManager.events[BROADCAST_EVENT].one_time = 0;
    // log_to_serial("Event Manager initialized: BROADCAST_EVENT");
    // log_decimal("Event size: ", eventManager.events[BROADCAST_EVENT].data_size);
    
}

// Registra un listener a un evento dado, en base al event id, el pid del proceso listener, un handler a ejecutar cuando el evento ocurra, y una condición.
// La condición es para limitar la escucha, por ejemplo en evento de teclado, solo escuchar la letra 'a', o en evento de muerte de un proceso, especificar el PID
// En principio cualquiera puede registrar una subscripción a un evento, pero si no tiene permisos de escucha del evento no se le notificará nada
void registerEventSubscription(int eventId, Pid pid, void (*handler)(void* data), void* condition_data) {
    if (eventId < 0 || eventId >= MAX_EVENTS) {
        return; // Invalid event ID
    }

    Listener* newListener = (Listener*) malloc(sizeof(Listener));
    if (!newListener) {
        log_to_serial("E: EventManager: Memory allocation failed for new listener");
        return; // Memory allocation failed
    }
    Pid pid_to_notify = getProcessGroupMain(pid);
    // console_log("I: EventManager: Registering event subscription for PID %d on event %d", pid_to_notify, eventId);

    newListener->pid = pid_to_notify;
    newListener->handler = handler;
    newListener->data = NULL; // No data for subscriptions
    if (condition_data != NULL){
        newListener->condition_data = malloc(eventManager.events[eventId].condition_data_size);
        if (!newListener->condition_data) {
            free(newListener); // Free the listener if condition data allocation fails
            log_to_serial("E: EventManager: Memory allocation failed for condition data");
            return; // Memory allocation failed
        }
        memcpy(newListener->condition_data, condition_data, eventManager.events[eventId].condition_data_size);
       
    } else {
        newListener->condition_data = NULL; // No condition data for subscriptions
    }
    newListener->type = SUSCRIPTION;

    // Add the new listener to the linked list of listeners for the event
    Listener* current = eventManager.events[eventId].listeners;
    newListener->next = current;
    eventManager.events[eventId].listeners = newListener;       // Para que sea O(1) en vez de O(n) al agregar un listener (el señorito se quejaba de que era O(n))

    // Si se registró para escuchar la muerte de un proceso, y este proceso no existe, se notifica inmediatamente
    if(eventId == PROCESS_DEATH_EVENT) {
        Process process_to_listen = getProcess(pid_to_notify);

        if(process_to_listen.pid == 0) {
            // El proceso no existe, notificar inmediatamente
            log_to_serial("E: ##### EventManager: Notifying process death event immediately for PID");
            notifyEvent(NULL, PROCESS_DEATH_EVENT, &process_to_listen.pid, filterProcessDeathCondition);
        }
    }
}

void registerEventWaiting(int eventId, Pid pid, void* data, void* condition_data) {
    if (eventId < 0 || eventId >= MAX_EVENTS) {
        return; // Invalid event ID
    }

    Listener* newListener = (Listener*) malloc(sizeof(Listener));
    if (!newListener) {
        return; // Memory allocation failed
    }

    // log_to_serial("I: EventManager: Registering waiting event");

    newListener->pid = pid;
    newListener->handler = NULL; // No handler for waiting events
    newListener->data = data;
    if (condition_data != NULL){    
        // log_to_serial("I: EventManager: Has condition data");

        newListener->condition_data = malloc(eventManager.events[eventId].condition_data_size);
        if (!newListener->condition_data) {
            free(newListener); // Free the listener if condition data allocation fails
            return; // Memory allocation failed
        }
        if (eventId == SLEEP_EVENT){
            SleepCondition* sleep_condition = (SleepCondition*) condition_data;
            sleep_condition->millis += milliseconds_elapsed(); // Add the current time to the sleep condition
        }
        memcpy(newListener->condition_data, condition_data, eventManager.events[eventId].condition_data_size);
    } else {
        newListener->condition_data = NULL; // No condition data for waiting events
    }
    newListener->type = WAITING;

    // Add the new listener to the linked list of listeners for the event
    Listener* current = eventManager.events[eventId].listeners;
    newListener->next = current;
    eventManager.events[eventId].listeners = newListener;       // Para que sea O(1) en vez de O(n) al agregar un listener (el señorito se quejaba de que era O(n))

    // log_to_serial("W: ------ EventManager: Registering waiting event");

    // log_to_serial("I: EventManager: Setting process as waiting");
    // log_decimal("I: EventManager: PID: ", pid);
    setWaiting(pid); // Set the process as waiting, so it can be woken up later when the event occurs

    // log_to_serial("E: EventManager: Le chupo un webo el wait... no espero una chota");
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
            free(current->condition_data); // Free the condition data if it was allocated
            free(current); // Free the memory allocated for the listener
            return; // Exit after removing the listener
        }
        previous = current;
        current = current->next;
    }
}

/**
 * Notify all listeners of a specific event.
 * 
 * @param pid The PID of the process to notify, or NULL to notify all processes.
 * @param eventId The ID of the event to notify.
 * @param data Pointer to the data associated with the event.
 * @param filter Optional filter function to apply to the condition data of each listener.
 */
void notifyEvent(Pid pid, int eventId, void* data, int (*filter)(void* condition_data, void* data)) {
    if (eventId < 0 || eventId >= MAX_EVENTS) {
        return; // Invalid event ID
    }
    // console_log("I: EventManager: Notifying event %d", eventId);

    Listener* current = eventManager.events[eventId].listeners;
    Listener* previous = NULL;
    while (current != NULL) {
        // console_log("I: EventManager: Checking listener with PID %d for event %d", current->pid, eventId);
        if (pid != NULL && !isSameProcessGroup(current->pid, pid)) {
            // If the PID does not match, skip this listener
            current = current->next;
            previous = current;
            continue;
        }
        if (filter != NULL && !filter(current->condition_data, data)) {
            // If a filter is provided and it returns false, skip this listener
            // console_log("I: EventManager: Skipping listener with PID %d for event %d due to filter", current->pid, eventId);
            current = current->next;
            previous = current;
            continue;
        }
        if (current->type == SUSCRIPTION) {
            void *eventData = malloc(eventManager.events[eventId].data_size);
            if (!eventData) {
                // Handle error: memory allocation failed
                // console_log("E: EventManager: Memory allocation failed for event data");
                continue;
            }  
            // Copy the data to the eventData buffer
            memcpy(eventData, data, eventManager.events[eventId].data_size);
            
            // Create a thread to handle the event
            Pid pid = newThread(current->handler, eventData, PRIORITY_NORMAL, current->pid);
            if (pid == 0) {
                // Handle error: could not create thread
                // console_log("E: EventManager: Could not create thread for event handler");
                free(eventData); // Free the allocated memory for event data
                
            } else {
                // console_log("I: EventManager: Created thread with PID %d to handle event %d, for process %d", pid, eventId, current->pid);
                // Successfully created thread to handle the event
                if (eventManager.events[eventId].one_time) {
                    // If the event is one-time, remove it from the list
                    if (previous == NULL) {
                        // Removing the first listener in the list
                        eventManager.events[eventId].listeners = current->next;
                        free(current->condition_data);  // Free the condition data if it was allocated
                        free(current);                  // Free the memory allocated for the listener
                        current = eventManager.events[eventId].listeners; // Move to next listener
                        continue; 
                    } else {
                        // Removing a listener in the middle or end of the list
                        previous->next = current->next;
                        free(current->condition_data); // Free the condition data if it was allocated
                        free(current); // Free the memory allocated for the listener
                        current = previous->next; // Move to next listener
                        previous = previous->next; // Update previous to the next listener
                        continue;
                    }
                }
            }
        } else if (current->type == WAITING) {
            // Notify to the scheduler that the process is not waiting anymore
            // Si el proceso dejó un puntero no nulo, es para que le copie la data del evento
            if(current->data != NULL) {
                memcpy(current->data, data, eventManager.events[eventId].data_size);
            }
            wakeProcess(current->pid);

            if (previous == NULL) {
                // Removing the first listener in the list
                eventManager.events[eventId].listeners = current->next;
                free(current->condition_data); // Free the condition data if it was allocated
                free(current); // Free the memory allocated for the listener
                current = eventManager.events[eventId].listeners; // Move to next listener
                continue; // Skip the rest of the loop to avoid double incrementing current
            } else {
                // Removing a listener in the middle or end of the list
                previous->next = current->next;
                free(current->condition_data); // Free the condition data if it was allocated
                free(current); // Free the memory allocated for the listener
                current = previous->next; // Move to next listener
                previous = previous->next; // Update previous to the next listener
                continue; // Skip the rest of the loop to avoid double incrementing current
            }
            
        }
        previous = current;
        current = current->next;
    }
}


int filterProcessDeathCondition(void* condition_data, void* data) {
    if (condition_data == NULL || data == NULL) {
        return 1; // No condition, accept all events
    }
    ProcessDeathCondition condition = *(ProcessDeathCondition*)condition_data;
    Pid pid = *(Pid*)data;
    // console_log("I: EventManager: Filtering process death condition for PID: %d", pid);
    // console_log("I: EventManager: Condition PID: %d", condition.pid);
    return (pid == condition.pid); // Filter by PID
}

void handleProcessDeath(Pid pid) {
    // console_log("Handling process death event for PID: %d", pid);
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
                free(current->condition_data); // Free the condition data if it was allocated
                free(current); // Free the memory allocated for the listener
                current = (previous == NULL) ? eventManager.events[i].listeners : previous->next; // Move to next listener
            } else {
                previous = current;
                current = current->next;
            }
        }
    }
    // console_log("I: EventManager: Notifying process death event for PID: %d", pid);

    notifyEvent(NULL, PROCESS_DEATH_EVENT, &pid, filterProcessDeathCondition);
}


int filterSleepCondition(void* condition_data, void* data) {
    if (condition_data == NULL || data == NULL) {
        return 1; // No condition, accept all events
    }
    SleepCondition* condition = (SleepCondition*)condition_data;
    uint64_t* millis = (uint64_t*)data;
    return (*millis >= condition->millis); // Filter by milliseconds
}

void handleSleep(uint64_t millis) {
    notifyEvent(NULL, SLEEP_EVENT, &millis, filterSleepCondition);
}

int filterRTCCondition(void* condition_data, void* data) {
    if (condition_data == NULL || data == NULL) {
        return 1; // No condition, accept all events
    }
    RTCCondition* condition = (RTCCondition*)condition_data;
    RTC_Time* time = (RTC_Time*)data;
    return (time->seconds == condition->seconds && time->minutes == condition->minutes && time->hours == condition->hours); // Filter by time
}

void handleRTCEvent(RTC_Time time) {
    notifyEvent(NULL, RTC_EVENT, &time, filterRTCCondition);
}

int filterKeyboardCondition(void* condition_data, void* data) {
    if (condition_data == NULL || data == NULL) {
        return 1; // No condition, accept all events
    }
    KeyboardCondition* condition = (KeyboardCondition*)condition_data;
    KeyboardEvent* event = (KeyboardEvent*)data;
    return (event->ascii == condition->ascii || event->scan_code == condition->scan_code); // Filter by ASCII or scancode character
}

void handleKeyEvent(KeyboardEvent keyEvent) {
    Pid process_with_focus = getFocusedWindow();
    notifyEvent(process_with_focus, KEY_EVENT, &keyEvent, filterKeyboardCondition);
}

int filterExceptionCondition(void* condition_data, void* data) {
    if (condition_data == NULL || data == NULL) {
        return 1; // No condition, accept all events
    }
    ExceptionCondition* condition = (ExceptionCondition*)condition_data;
    Exception* exception = (Exception*)data;
    return (exception->exception_id == condition->exception_id); // Filter by exception ID
}

void handleException(Exception exception) {
    notifyEvent(NULL, EXCEPTION_EVENT, &exception, filterExceptionCondition);
}

