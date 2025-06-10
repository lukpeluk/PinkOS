#include <drivers/serialDriver.h>
#include <processManager/scheduler.h>
#include <processManager/processState.h>
#include <stdint.h>
#include <stdlib.h>
#include <memoryManager/memoryManager.h>
#include <windowManager/windowManager.h>

#define STACK_SIZE 0x1000       // Tamaño de cada stack (4 KB)
#define TICKS_TILL_SWITCH 10     // Cantidad de ticks hasta cambiar de proceso

#define NULL 0


typedef struct Semaphore {
    uint64_t id;                    // ID único del semáforo
    int value;                      // Contador del semáforo
    struct Semaphore *next;         // Siguiente semáforo (lista null terminated)
} Semaphore;


typedef struct ProcessControlBlock {
    Process process;
    uint32_t quantum;               // Cantidad de ticks que el proceso puede ejecutar antes de ser interrumpido
    uint64_t stackBase;             // Dirección base del stack asignado, no es lo mismo que el rbp guardado ya que eso puede ser modificado por el proceso
    Registers registers;            // Registros del proceso, incluyendo el stack pointer (que en realidad apunta al interrupt stack frame, no al stack del proceso en sí)
    struct ProcessControlBlock * parent;
    struct ProcessControlBlock *next; // Siguiente proceso (lista circular)
    Semaphore * waiting_for;          // Semáforo que el proceso está esperando, NULL si no tiene
} ProcessControlBlock;


extern uint64_t load_interrupt_frame(InterruptStackFrame *frame, uint64_t stack_pointer);
extern void magic_recover(Registers *registers);
extern void magic_recover_old(InterruptStackFrame *cri, uint64_t args);
extern void push_to_custom_stack_pointer(uint64_t stack_pointer, uint64_t value_to_push);

static ProcessControlBlock *currentProcessBlock = NULL;  // Proceso actualmente en ejecución
static ProcessControlBlock *processList = NULL;     // Lista circular de procesos
static ProcessControlBlock *processListTail = NULL; 
static Pid nextPID = 1;       // Contador para asignar PIDs únicos

static Semaphore *firstSemaphore = NULL;
static uint64_t nextSemaphoreId = 1; // Contador para asignar IDs únicos a los semáforos

static uint32_t ticksSinceLastSwitch = 0; // Contador de ticks desde el último cambio de proceso



void initScheduler() {
//     log_to_serial("initScheduler: Iniciando el scheduler");
    processList = NULL;
    currentProcessBlock = NULL;
    firstSemaphore = NULL; 
}

// Mayor quantum significa que el proceso tiene más tiempo para ejecutarse
uint32_t getQuantumByPriority(Priority priority) {
    // Asignar quantum según la prioridad del proceso
    switch (priority) {
        case PRIORITY_LOW:
            return 2; 
        case PRIORITY_NORMAL:
            return 5; 
        case PRIORITY_HIGH:
            return 10; 
        default:
            return 5; 
    }
}


Pid getCurrentProcessPID() {
    if (currentProcessBlock == NULL) {
        return 0; // No hay proceso actual
    }
    return currentProcessBlock->process.pid;
}

// Pid 0 es inválido, por eso se devuelve un proceso con pid 0 si no hay proceso actual
// En ese caso todo lo demás es basura y debe ignorarse
Process getProcess(Pid pid) {
    Process invalid_process = {.pid = 0};

    if(processList == NULL) return invalid_process;

    ProcessControlBlock * current = processList;
    do {
        if(current->process.pid == pid){
            return current->process; // Devuelve el proceso actual, NULL si no hay ninguno
        }
        current = current->next;
    } while (current != processList);

    return invalid_process;
    
}


ProcessControlBlock * getProcessControlBlock(Pid pid){
    if(processList == NULL) return NULL;

    ProcessControlBlock * current = processList;
    do {
        if(current->process.pid == pid){
            return current; // Devuelve el proceso actual, NULL si no hay ninguno
        }
        current = current->next;
    } while (current != processList);

    return NULL; 
}

Process getParent(Pid pid){
    ProcessControlBlock * process = getProcessControlBlock(pid);
    if(process == NULL || process->parent == NULL) {
        return (Process){.pid = 0}; // No hay padre, devuelve un proceso inválido
    }
    return process->parent->process; // Devuelve el proceso padre
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



void quitWrapper(){
//     log_to_serial("quitWrapper: Programa saliendo naturalmente");
    terminateProcess(getCurrentProcessPID()); // Terminar el proceso actual
}

// Agrega un nuevo proceso al planificador, no lo ejecuta inmediatamente
// Le asigna un PID, inicializa el stack y los registros, y lo agrega a la lista de procesos
ProcessControlBlock * addProcessToScheduler(Program program, ProgramEntry entry, char *arguments, ProcessType type, Priority priority, ProcessControlBlock *parent) {

//     log_to_serial("addProcessToScheduler: Iniciando la creacion de un nuevo proceso");

    static uint32_t processCount = 0; // Contador de procesos creados
    if (program.entry == NULL, entry == NULL) {
//         log_to_serial("addProcessToScheduler: Error, invalid input");
        return NULL;
    }
//     log_to_serial("addProcessToScheduler: Agregando proceso");

    if( parent == NULL && processList != NULL) {
//         log_to_serial("addProcessToScheduler: Init ya existe, error!");
        return NULL; // No se puede crear un proceso sin padre si ya hay un init
    }
        

    // Allocar memoria para el PCB usando malloc
    // Se guardan los datos del programa, se asigna un pid, y se inicializan los punteros
    // El stack se alloca dinámicamente usando malloc
    ProcessControlBlock *newProcessBlock = (ProcessControlBlock *)allocateProcessMemory(sizeof(ProcessControlBlock));

    if (newProcessBlock == NULL) {
//         log_to_serial("addProcessToScheduler: Error al alocar memoria para el PCB");
        return NULL; // Error al alocar memoria
    }
    
    newProcessBlock->process.program = program; // Nombre del programa
    newProcessBlock->process.pid = nextPID++;
//     log_to_serial("addProcessToScheduler : Asignando PID al nuevo proceso");
//     log_decimal(">>>>>>>>>>>>>>>>>>>>>>>>>>>> . addProcessToScheduler: PID asignado: ", newProcessBlock->process.pid);

    newProcessBlock->process.type = type;                       // Tipo de proceso (normal, gráfico, etc.)
    newProcessBlock->process.state = PROCESS_STATE_NEW;         // Estado inicial del proceso
    newProcessBlock->process.priority = priority;
    newProcessBlock->parent = parent;                           // Guardar el padre del proceso, si es que tiene uno
    newProcessBlock->quantum = getQuantumByPriority(priority);  // Cantidad de ticks que el proceso puede ejecutar antes de ser interrumpido
    newProcessBlock->waiting_for = NULL;                        // Inicializar el semáforo de espera como NULL
    
    newProcessBlock->stackBase = allocateStack(processCount);           // Asignar stack predefinido
    newProcessBlock->registers.rsp = newProcessBlock->stackBase - 8;    // Inicializar stack pointer y restar lo que se va a usar para el ret a quitProgram
    newProcessBlock->registers.rdi = (uint64_t)arguments;               // Guardar el argumento en los registros del proceso, el resto de los registros son basura


    if (newProcessBlock->stackBase == NULL) {
//         log_to_serial("addProcessToScheduler: Error al alocar memoria para el stack del proceso");
        free(newProcessBlock); // Liberar el PCB si no se pudo alocar el stack
        return NULL; // Error al alocar memoria para el stack
    }

    // Guardo un puntero a la función de salida del programa, que es quitProgram (por eso rsp - 8)
    push_to_custom_stack_pointer(newProcessBlock->stackBase, (uint64_t)quitWrapper);

    // Generar el CRI inicial para el proceso
    // El rip se inicializa al entry point del programa, y el rsp al stack pointer del proceso antes de cargar el interrupt stack frame
    InterruptStackFrame cri = getDefaultCRI();
    cri.rip = (uint64_t) entry; // Entry point del programa
    cri.rsp = newProcessBlock->registers.rsp; // Usar el stack pointer del proceso

//     log_hex("addProcessToScheduler: process stack base: ", newProcessBlock->stackBase);
//     log_hex("addProcessToScheduler: process initial rsp: ", newProcessBlock->registers.rsp);
//     log_hex("addProcessToScheduler: process rip: ", cri.rip);
//     log_decimal("addProcessToScheduler: process pid: ", newProcessBlock->process.pid);

    // cargar en el stack del proceso el cri generado, y actualizar el stack pointer para que apunte al interrupt stack frame
    newProcessBlock->registers.rsp = load_interrupt_frame(&cri, newProcessBlock->registers.rsp);

//     log_hex("addProcessToScheduler: process rsp after loading cri: ", newProcessBlock->registers.rsp);

    // Los procesos son listas circulares, round robin básico
    // Si es el primer proceso, inicializar la lista (solo pasaría con el init)
    if (processList == NULL) {
        processList = newProcessBlock;
        // currentProcessBlock = newProcessBlock; // Como es el primer proceso, lo hacemos el actual
    }
    processListTail->next = newProcessBlock;  
    newProcessBlock->next = processList;      
    processListTail = newProcessBlock;        

    processCount++;
//     log_to_serial("addProcessToScheduler: Proceso agregado con exito");
    return newProcessBlock; // Retornar el nuevo proceso agregado
}


Pid newProcess(Program program, char *arguments, Priority priority, Pid parent_pid) {
    // para debug
    if(parent_pid == 0) {
//         log_to_serial("newProcess: Creando nuevo proceso sin padre, Init");
    } else {
//         log_to_serial("newProcess: Creando nuevo proceso con padre");
    }

    if(parent_pid == 0 && processList != NULL) {
//         log_to_serial("newProcess: Init ya existe, error!");
        return 0; // No se puede crear un proceso sin padre si ya hay un init
    }

    ProcessControlBlock * parent = getProcessControlBlock(parent_pid);
    if(parent == NULL  && processList != NULL){
//         log_to_serial("invalid parent process");
        return NULL;
    }
    if(parent->process.type != PROCESS_TYPE_MAIN){
        parent = getProcessControlBlock(parent->parent);
    }

    ProcessControlBlock * newProcessBlock = addProcessToScheduler(program, program.entry, arguments, PROCESS_TYPE_MAIN, priority, parent);

    if (IS_GRAPHIC(newProcessBlock)) {
        uint8_t *buffer = addWindow(newProcessBlock->process.pid);
        if (buffer == NULL) {
//             log_to_serial("addProcessToScheduler: Error al agregar la ventana del proceso grafico");
            return 0;
        }
    }

//     log_to_serial("newMainProcess: Agregando nuevo proceso al scheduler");
//     log_decimal("newMain with PID: ", newProcessBlock->process.pid);
    return newProcessBlock->process.pid;
}

Pid newThread(ProgramEntry entrypoint, char *arguments, Priority priority, Pid parent_pid) {
    // Tengo que validar que el parent sea un proceso main y no un thread sino tengo que crear el thread a nombre del main del thread

    ProcessControlBlock * parent = getProcessControlBlock(parent_pid);
    if(parent == NULL){
//         log_to_serial("invalid parent process");
        return NULL;
    }
    if(parent->process.type != PROCESS_TYPE_MAIN){
        parent = getProcessControlBlock(parent->parent);
    }

    // le saco permisos gráficos al thread
    Program threadProgram = parent->process.program;
    // threadProgram.permissions &= ~DRAWING_PERMISSION; // Quitar permisos gráficos al thread
    ProcessControlBlock * newProcessBlock = addProcessToScheduler(threadProgram, entrypoint, arguments, PROCESS_TYPE_THREAD, priority, parent);
    
//     log_to_serial("newThread: Agregando nuevo thread al scheduler:");
//     log_decimal("newThread with PID: ", newProcessBlock->process.pid);
    return newProcessBlock->process.pid;
}


//! NO USAR DIRECTAMENTE, USAR terminateProcess QUE ES RECURSIVA, ESTO PODRÍA DEJAR PROCESOS HUÉRFANOS
//! Cuando se llame, se da por sentado que el proceso existe y puede matarse, las validaciones deben hacerse antes de llamar a esta función
int terminateSingleProcess(uint32_t pid) {

    ProcessControlBlock * to_remove = getProcessControlBlock(pid);
    if(to_remove == NULL){
//         log_to_serial("invalid parent process");
        return -1;
    }
    if(to_remove->process.pid == 1 || to_remove->next == to_remove){
        // O estás borrando init o estás borrando el único proceso que hay, ilegalísimo
        return -2;
    }

    // Si el proceso es gráfico, eliminar la ventana asociada
    uint64_t was_graphic = IS_GRAPHIC(to_remove);
    if(was_graphic) {
//         log_to_serial("terminateCurrentProcess: El proceso actual es grafico, eliminando ventana asociada");
        removeWindow(pid); 
    }

    // Marcarlo como terminado para que el scheduler lo elimine
    to_remove->process.state = PROCESS_STATE_TERMINATED;

    handleProcessDeath(to_remove->process.pid); // Enviar evento de muerte

}


// Matar un proceso borra su ventana, avisa que murió con un evento, y lo marca como terminado para que el bucle del scheduler lo elimine
// Es recursivo para así matar también a los hijos
int terminateProcess(Pid pid) {
    // log_to_serial("removeProcessFromScheduler: Eliminando proceso y sus hijos");

    ProcessControlBlock * to_remove = getProcessControlBlock(pid);
    if(to_remove == NULL){
//         log_to_serial("invalid parent process");
        return -1;
    }
    if(to_remove->process.pid == 1 || to_remove->next == to_remove){
        // O estás borrando init o estás borrando el único proceso que hay, ilegalísimo
        return -2;
    } 

    runOnChilds(terminateSingleProcess, pid); // Ejecutar el callback recursivamente en todos los descendientes del proceso especificado
    // log_to_serial("terminateProcess: Proceso y sus hijos eliminados");

    terminateSingleProcess(pid); // Terminar el proceso actual

    // Si el proceso actual fue eliminado, ya programar el siguiente proceso (no voy a poder volver de esta función)
    if(currentProcessBlock->process.state == PROCESS_STATE_TERMINATED) {
        scheduleNextProcess();
    }
    return 0; // Proceso eliminado exitosamente
}



void scheduleNextProcess() {
    // log_to_serial("I: scheduleNextProcess: Programando el siguiente proceso");

    if (currentProcessBlock == NULL) return;

    currentProcessBlock->process.state = currentProcessBlock->process.state == PROCESS_STATE_RUNNING ? PROCESS_STATE_READY : currentProcessBlock->process.state; // Cambiar el estado del proceso actual a READY

    ProcessControlBlock *nextProcess = currentProcessBlock->next;
    ProcessControlBlock * current = nextProcess;
    ProcessControlBlock * prev = currentProcessBlock; // Empezar desde el final de la lista para poder eliminar el proceso actual si es necesario
    currentProcessBlock = nextProcess; // Actualizar el proceso actual al siguiente

    // Pasa de largo los que no están en estado READY o NEW
    while (current->process.state != PROCESS_STATE_READY && current->process.state != PROCESS_STATE_NEW) {
        if(current->process.state == PROCESS_STATE_TERMINATED) {
            nextProcess = current->next; // Guardar el siguiente proceso antes de liberar el actual

            // Caso especial: borre la tail o head de la lista
            if (current == processList) {
                // Si el proceso eliminado era el head, actualizar el head
                processList = nextProcess;
            }
            if (current == processListTail) {
                // Si el proceso eliminado era la tail, actualizar la tail
                processListTail = prev;
            }
            // log_to_serial("scheduleNextProcess: El proceso actual ya esta terminado, no se puede programar otro proceso");
            free(current->stackBase); // Liberar el stack del proceso actual
            free(current); // Liberar el PCB del proceso actual


            prev->next = nextProcess; // Eliminar el proceso actual de la lista
            current = nextProcess;
            continue;
        } 

        // vuelvo a llegar, no había ningún proceso en estado READY o NEW
        if (current == currentProcessBlock) {
            // log_to_serial("scheduleNextProcess: No hay procesos en estado READY");
            return; // No hay procesos en estado READY ni NEW, no se puede programar otro proceso
        }

        current = current->next;
        prev = prev->next; // Avanzar al siguiente proceso en la lista
    }
    currentProcessBlock = current; // Actualizar el proceso actual al siguiente que está en estado READY o NEW

    if (currentProcessBlock->process.state == PROCESS_STATE_NEW) {
        // Si el proceso es nuevo, inicializarlo
        currentProcessBlock->process.state = PROCESS_STATE_READY;
        // log_to_serial("scheduleNextProcess: Proceso nuevo, inicializando");
    }

    currentProcessBlock->process.state = PROCESS_STATE_RUNNING; // Cambiar el estado del nuevo proceso a RUNNING

    // Actualizar el estado del kernel, indicando qué proceso está en ejecución, los permisos, y de-escalando los privilegios de kernel
    desactivateRootMode();

    // LOGS    
    // log_to_serial("I: scheduleNextProcess: Proceso actual:");
    // log_to_serial(currentProcessBlock->process.program.name);
    log_decimal("I: scheduleNextProcess: PID: ", currentProcessBlock->process.pid);
    // log_to_serial("scheduleNextProcess: Restaurando registros del proceso actual con magic_recover");

    // log_hex("scheduleNextProcess: Stack base del proceso actual: ", currentProcessBlock->stackBase);
    // log_hex("scheduleNextProcess: RSP del proceso actual: ", currentProcessBlock->registers.rsp);

    // log_to_serial(">>>>>>>>>>>>>>>> scheduleNextProcess: YENDO A MAGIC RECOVER");
    magic_recover(&currentProcessBlock->registers);
}


// Bucle del planificador, a ejecutar lo frecuentemente que se quiera, ej. cada timertick
// Bucle del planificador, a ejecutar lo frecuentemente que se quiera, ej. cada timertick
void schedulerLoop() {

    // Solo schedulea si hay procesos y pasó el quantum del proceso
    ticksSinceLastSwitch++;
    if (processList == NULL || ticksSinceLastSwitch % TICKS_TILL_SWITCH != 0)
        return;

    if(currentProcessBlock == NULL){
        currentProcessBlock = processList;
    }

    scheduleNextProcess();
}


// Guardar el contexto del proceso actual en base al backup de registros, para poder restaurarlo después
// La idea es que en cada interrupción se ejecute
void backupCurrentProcessRegisters() {
    // Guardar los registros del proceso actual en el backup
    if (currentProcessBlock == NULL || currentProcessBlock->process.state != PROCESS_STATE_RUNNING) return; // No hay proceso actual
    BackupRegisters * backup = getBackupRegisters();
    currentProcessBlock->registers = backup->registers;
}



// Ejecuta un callback recursivamente en todos los descendientes del proceso especificado
// No corre en el parent, solo en los descendientes
void runOnChilds(void (*callback)(ProcessControlBlock *), Pid parent_pid) {
    if (processList == NULL || callback == NULL) {
//         log_to_serial("runOnChilds: Lista de procesos vacia o callback invalido");
        return;
    }

    // Buscar todos los hijos directos del parent_pid
    ProcessControlBlock *current = processList;
    do {
        if (current->parent != NULL && current->parent->process.pid == parent_pid) {
            // Encontramos un hijo, recursivamente procesar sus hijos primero
            runOnChilds(callback, current->process.pid);
            
            // Después de procesar todos los descendientes, ejecutar callback en este proceso
            callback(current);
        }
        current = current->next;
    } while (current != processList);
}



// ------ SEMÁFOROS Y ESPERAS ------


// Deja el proceso en espera
// Si se waitea el actual, no se vuelve de esta función
// En caso de exito, devuelve 0, en caso de error -1
int setWaiting(Pid pid) {
    ProcessControlBlock *current = processList;
    if (current == NULL) {
        return -1; // Error: no hay procesos en la lista
    }

    do {
        if (current->process.pid == pid) {
            // Cambiar el estado del proceso a WAITING
            current->process.state = PROCESS_STATE_WAITING;

            if(current == currentProcessBlock) {
                // Si el proceso actual es el que se está poniendo en espera, programar el siguiente proceso
                scheduleNextProcess(); // Cambiar al siguiente proceso
            }
            return 0; // Éxito
        }
        current = current->next;
    }
    while (current != processList);

    return -1; // Error: proceso no encontrado
}


 // Despierta un proceso que estaba en espera
 // En caso de éxito devuelve 0, en caso de error -1
int wakeProcess(Pid pid) {
    ProcessControlBlock *current = processList;
    if (current == NULL) {
        return -1; // Error: no hay procesos en la lista
    }

    do {
        if (current->process.pid == pid) {
            // Cambiar el estado del proceso a WAITING
            current->process.state = PROCESS_STATE_READY;
            return 0; // Éxito
        }
        current = current->next;
    }
    while (current != processList);

    return -1; // Error: proceso no encontrado
}



// Función interna
// null si no lo encuentra
Semaphore * getSemaphore(uint64_t id) {
    Semaphore *sem = firstSemaphore;
    while (sem != NULL) {
        if (sem->id == id) {
            return sem; // Encontré el semáforo con el ID especificado
        }
        sem = sem->next; // Avanzar al siguiente semáforo
    }
    return NULL; // No encontré el semáforo con el ID especificado
}


void sem_init(uint64_t id, int initial_value) {
    Semaphore* sem = (Semaphore*)malloc(sizeof(Semaphore));

    sem->id = nextSemaphoreId++; 
    sem->value = initial_value;
    sem->next = firstSemaphore;
    firstSemaphore = sem; 
}


void sem_destroy(uint64_t id) {
    Semaphore *sem = getSemaphore(id);
    if (sem == NULL) {
        return; // No se encontró el semáforo, no hay nada que destruir
    }

    // validar que nadie esté esperando este semáforo
    ProcessControlBlock *current = processList;
    do {
        if (current->waiting_for == sem) {
            // Hay un proceso esperando este semáforo, no se puede destruir
            return; 
        }
        current = current->next;
    } while (current != processList);

    // Buscar el semáforo en la lista y eliminarlo
    Semaphore *currentSem = firstSemaphore;
    Semaphore *prev = NULL;
    while (currentSem != NULL) {
        if (currentSem->id == id) {
            // Encontré el semáforo a destruir
            if (prev == NULL) {
                // Es el primer semáforo
                firstSemaphore = currentSem->next; // Actualizar la cabeza de la lista
            } else {
                prev->next = currentSem->next; // Eliminar el semáforo de la lista
            }
            free(currentSem); // Liberar la memoria del semáforo
            return; // Salir después de destruir el semáforo
        }
        prev = currentSem;
        currentSem = currentSem->next;
    }
}


// Claramente esta función no vuelve, el proceso queda en espera
void sem_wait(uint64_t id) {
    Semaphore *sem = getSemaphore(id);

    if(sem == NULL || currentProcessBlock == NULL) {
        return;
    }

    sem->value--;

    if (sem->value < 0) {
        // No uso setWaiting porque es una función externa, iteraría al pedo
        currentProcessBlock->process.state = PROCESS_STATE_WAITING; 
        scheduleNextProcess();
    }
}

void sem_post(uint64_t id) {
    Semaphore *sem = getSemaphore(id);

    if(sem == NULL) {
        return;
    }

    sem->value++;

    if (sem->value <= 0) {
        // Desencolo un proceso que estaba esperando este semáforo
        ProcessControlBlock *current = processList;
        do {
            if (current->waiting_for == sem) {
                // Despertar el proceso
                current->process.state = PROCESS_STATE_READY;
                current->waiting_for = NULL; // Limpiar el semáforo que estaba esperando
                return; // Salir después de despertar un proceso (no se despierta a todos)
            }
            current = current->next;
        } while (current != processList);
    }
}
