#include <drivers/serialDriver.h>
#include <scheduling/scheduler.h>
#include <processState.h>
#include <stdint.h>
#include <stdlib.h>
#include <memoryManager/memoryManager.h>
#include <windowManager/windowManager.h>

#define STACK_SIZE 0x1000       // Tamaño de cada stack (4 KB)
#define TICKS_TILL_SWITCH 10     // Cantidad de ticks hasta cambiar de proceso

#define NULL 0

extern uint64_t load_interrupt_frame(InterruptStackFrame *frame, uint64_t stack_pointer);
extern void magic_recover(Registers *registers);
extern void magic_recover_old(InterruptStackFrame *cri, uint64_t args);
extern void push_to_custom_stack_pointer(uint64_t stack_pointer, uint64_t value_to_push);

static ProcessControlBlock *currentProcess = NULL;  // Proceso actualmente en ejecución
static ProcessControlBlock *processList = NULL;     // Lista circular de procesos
static ProcessControlBlock *processListTail = NULL; 
static uint32_t nextPID = 1;       // Contador para asignar PIDs únicos

static uint32_t ticksSinceLastSwitch = 0; // Contador de ticks desde el último cambio de proceso

uint32_t getCurrentProcessPID() {
    if (currentProcess == NULL) {
        return 0; // No hay proceso actual
    }
    return currentProcess->pid;
}

void initScheduler() {
    log_to_serial("initScheduler: Iniciando el scheduler");
    processList = NULL;
    currentProcess = NULL;
}

ProcessControlBlock *allocateProcessMemory(size_t size) {
    return (ProcessControlBlock *)malloc(sizeof(ProcessControlBlock));
}

uint64_t allocateStack(uint32_t processIndex) {
    // Allocar STACK_SIZE bytes usando malloc
    void* stackMemory = malloc(STACK_SIZE);
    if (stackMemory == NULL) {
        return 0; // Error: no se pudo allocar memoria para el stack
    }
    
    // El stack crece hacia abajo, así que el stack base debe ser el final de la memoria allocada
    return (uint64_t)stackMemory + STACK_SIZE;
}


void itoa(int value, char *str, int base) {
    char *p = str;
    int num = value;
    int sign = 0;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        sign = 1;
        num = -value;
    }

    char buf[32];
    int i = 0;

    while (num != 0) {
        int digit = num % base;
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num /= base;
    }

    if (sign)
        *p++ = '-';

    while (i--)
        *p++ = buf[i];

    *p = '\0';
}

// Funciones auxiliares para logging
void log_hex(char* prefix, uint64_t value) {
    log_to_serial(prefix);
    char hexStr[20];
    log_to_serial("0x");
    itoa(value, hexStr, 16);
    log_to_serial(hexStr);
}

void log_decimal(char* prefix, uint32_t value) {
    log_to_serial(prefix);
    char decStr[12];
    itoa(value, decStr, 10);
    log_to_serial(decStr);
}

void log_string(char* message) {
    log_to_serial(message);
}


void quitWrapper(){
    log_to_serial("quitWrapper: Programa saliendo naturalmente");
    quitProgram();
}

// Agrega un nuevo proceso al planificador, no lo ejecuta inmediatamente
void addProcessToScheduler(Program *program, char *arguments) {
    static uint32_t processCount = 0; // Contador de procesos creados
    if (program == NULL || program->entry == NULL) {
        log_to_serial("addProcessToScheduler: Error, programa o entry no válido");
        return;
    }
    log_to_serial("addProcessToScheduler: Agregando proceso");

    // Allocar memoria para el PCB usando malloc
    // Se guardan los datos del programa, se asigna un pid, y se inicializan los punteros
    // El stack se alloca dinámicamente usando malloc
    ProcessControlBlock *newProcess = (ProcessControlBlock *)allocateProcessMemory(sizeof(ProcessControlBlock));
    newProcess->name = program->name;
    newProcess->pid = nextPID++;
    newProcess->permissions = program->perms;
    newProcess->stackBase = allocateStack(processCount); // Asignar stack predefinido
    newProcess->registers.rsp = newProcess->stackBase - 8; // Inicializar stack pointer y restar lo que se va a usar para el ret a quitProgram

    // Guardo un puntero a la función de salida del programa, que es quitProgram (por eso rsp - 8)
    push_to_custom_stack_pointer(newProcess->stackBase, (uint64_t)quitWrapper);

    // Generar el CRI inicial para el proceso
    // El rip se inicializa al entry point del programa, y el rsp al stack pointer del proceso antes de cargar el interrupt stack frame
    InterruptStackFrame cri = getDefaultCRI();
    cri.rip = (uint64_t)program->entry;
    cri.rsp = newProcess->registers.rsp; // Usar el stack pointer del proceso

    log_hex("addProcessToScheduler: process stack base: ", newProcess->stackBase);
    log_hex("addProcessToScheduler: process initial rsp: ", newProcess->registers.rsp);
    log_hex("addProcessToScheduler: process rip: ", cri.rip);
    log_decimal("addProcessToScheduler: process pid: ", newProcess->pid);

    // cargar en el stack del proceso el cri generado, y actualizar el stack pointer para que apunte al interrupt stack frame
    newProcess->registers.rsp = load_interrupt_frame(&cri, newProcess->registers.rsp);

    log_hex("addProcessToScheduler: process rsp after loading cri: ", newProcess->registers.rsp);

    // Guardar el argumento en los registros del proceso, el resto de los registros son basura
    newProcess->registers.rdi = (uint64_t)arguments; 

    // Los procesos son listas circulares, round robin básico
    // Si es el primer proceso, inicializar la lista
    if (processList == NULL) {
        processList = newProcess;
        // currentProcess = newProcess; // Como es el primer proceso, lo hacemos el actual
    }
    processListTail->next = newProcess;  
    newProcess->next = processList;      
    processListTail = newProcess;        

    processCount++;
    log_to_serial("addProcessToScheduler: Proceso agregado con éxito");

    // agregar la ventana del proceso al window manager si es un proceso gráfico
    if (IS_GRAPHIC(newProcess)) {
        uint8_t *buffer = addWindow(newProcess->pid);
        if (buffer == NULL) {
            log_to_serial("addProcessToScheduler: Error al agregar la ventana del proceso gráfico");
        }
    }
}

// Elimina un proceso del planificador por su PID
// TODO: Implementar free() para liberar la memoria del PCB y stack cuando se termine el memory manager
// Por ahora free() no hace nada, pero al menos la memoria está siendo tracked por el memory manager
void removeProcessFromScheduler(uint32_t pid) {
    log_to_serial("removeProcessFromScheduler: Eliminando proceso");

    if (processList == 0) return;

    ProcessControlBlock *current = processList;
    ProcessControlBlock *prev = processListTail;
    do {
        if (current->pid == pid) {
            if (current == processList) {
                // Si es el primer proceso, actualizar la cabeza de la lista
                processList = current->next;
            } else if (current == processListTail) {
                // Si es el último proceso, actualizar la cola de la lista
                processListTail = prev;
            }

            // Si solo hay un proceso, reiniciar la lista
            if(prev == current) {
                processList = NULL;
                processListTail = NULL;
                currentProcess = NULL;
                return;
            }

            // eliminar de la lista
            prev->next = current->next;
            return;
        }
        prev = current;
        current = current->next;
    } while (current != processList);
    
}

void scheduleNextProcess() {
    log_to_serial("scheduleNextProcess: Programando el siguiente proceso");

    if (currentProcess == NULL) return;

    currentProcess = currentProcess->next;

    // Actualizar el estado del kernel, indicando qué proceso está en ejecución, los permisos, y de-escalando los privilegios de kernel
    desactivateRootMode();
    setPermissions(currentProcess->permissions);
    setCurrentProcess(currentProcess->name);

    log_string("scheduleNextProcess: Proceso actual:");
    log_string(currentProcess->name);
    log_decimal("scheduleNextProcess: PID: ", currentProcess->pid);
    log_string("scheduleNextProcess: Restaurando registros del proceso actual con magic_recover");

    log_hex("scheduleNextProcess: Stack base del proceso actual: ", currentProcess->stackBase);
    log_hex("scheduleNextProcess: RSP del proceso actual: ", currentProcess->registers.rsp);


    magic_recover(&currentProcess->registers);
}

void terminateCurrentProcess() {
    uint32_t pid = currentProcess->pid;
    uint64_t was_graphic = IS_GRAPHIC(currentProcess);

    if(was_graphic) {
        log_to_serial("terminateCurrentProcess: El proceso actual es gráfico, eliminando ventana asociada");
        removeWindow(pid); // Eliminar la ventana asociada al proceso gráfico
    }

    removeProcessFromScheduler(pid);

    if(currentProcess == NULL){
        setPermissions(ROOT_PERMISSIONS);
        setCurrentProcess((char *)SYSTEM_PROCESS);

        InterruptStackFrame cri = getDefaultCRI();
        cri.rip = (uint64_t)callRestoreContextHandler;
        cri.rsp = getSystemStackBase();

        magic_recover_old(&cri, was_graphic); // Restaurar el contexto del sistema
    } else {
        scheduleNextProcess();
    }
}


// Bucle del planificador, a ejecutar lo frecuentemente que se quiera, ej. cada timertick
void schedulerLoop() {
    // Si no hay procesos, no hacer nada
    ticksSinceLastSwitch++;
    if (processList == NULL || ticksSinceLastSwitch % TICKS_TILL_SWITCH != 0) {
        return;
    }

    log_to_serial("schedulerLoop: Ejecutando el bucle del scheduler");

    if(currentProcess == NULL){
        currentProcess = processList;
    } else {
        // Guardar el contexto del proceso actual en base al backup de registros, para poder restaurarlo después
        BackupRegisters * backup = getBackupRegisters();
        currentProcess->registers = backup->registers;
    }

    log_to_serial("schedulerLoop: pasando al siguiente proceso");

    scheduleNextProcess();
}