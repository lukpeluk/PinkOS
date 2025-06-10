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

typedef struct ProcessControlBlock {
    Process process;
    uint64_t stackBase;             // Dirección base del stack asignado, no es lo mismo que el rbp guardado ya que eso puede ser modificado por el proceso
    Registers registers;            // Registros del proceso, incluyendo el stack pointer (que en realidad apunta al interrupt stack frame, no al stack del proceso en sí)
    struct ProcessControlBlock * parent;
    struct ProcessControlBlock *next; // Siguiente proceso (lista circular)
} ProcessControlBlock;


extern uint64_t load_interrupt_frame(InterruptStackFrame *frame, uint64_t stack_pointer);
extern void magic_recover(Registers *registers);
extern void magic_recover_old(InterruptStackFrame *cri, uint64_t args);
extern void push_to_custom_stack_pointer(uint64_t stack_pointer, uint64_t value_to_push);

static ProcessControlBlock *currentProcessBlock = NULL;  // Proceso actualmente en ejecución
static ProcessControlBlock *processList = NULL;     // Lista circular de procesos
static ProcessControlBlock *processListTail = NULL; 
static uint32_t nextPID = 1;       // Contador para asignar PIDs únicos

static uint32_t ticksSinceLastSwitch = 0; // Contador de ticks desde el último cambio de proceso


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


void initScheduler() {
    log_to_serial("initScheduler: Iniciando el scheduler");
    processList = NULL;
    currentProcessBlock = NULL;
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
    log_to_serial("quitWrapper: Programa saliendo naturalmente");
    terminateProcess(getCurrentProcessPID()); // Terminar el proceso actual
}

// Agrega un nuevo proceso al planificador, no lo ejecuta inmediatamente
// Le asigna un PID, inicializa el stack y los registros, y lo agrega a la lista de procesos
ProcessControlBlock * addProcessToScheduler(Program program, ProgramEntry entry, char *arguments, ProcessType type, Priority priority, ProcessControlBlock *parent) {

    log_to_serial("addProcessToScheduler: Iniciando la creacion de un nuevo proceso");

    static uint32_t processCount = 0; // Contador de procesos creados
    if (program.entry == NULL, entry == NULL) {
        log_to_serial("addProcessToScheduler: Error, invalid input");
        return NULL;
    }
    log_to_serial("addProcessToScheduler: Agregando proceso");

    if( parent == NULL && processList != NULL) {
        log_to_serial("addProcessToScheduler: Init ya existe, error!");
        return NULL; // No se puede crear un proceso sin padre si ya hay un init
    }
        

    // Allocar memoria para el PCB usando malloc
    // Se guardan los datos del programa, se asigna un pid, y se inicializan los punteros
    // El stack se alloca dinámicamente usando malloc
    ProcessControlBlock *newProcessBlock = (ProcessControlBlock *)allocateProcessMemory(sizeof(ProcessControlBlock));

    if (newProcessBlock == NULL) {
        log_to_serial("addProcessToScheduler: Error al alocar memoria para el PCB");
        return NULL; // Error al alocar memoria
    }
    
    newProcessBlock->process.program = program; // Nombre del programa
    newProcessBlock->process.pid = nextPID++;
    log_to_serial("addProcessToScheduler : Asignando PID al nuevo proceso");
    log_decimal(">>>>>>>>>>>>>>>>>>>>>>>>>>>> . addProcessToScheduler: PID asignado: ", newProcessBlock->process.pid);

    newProcessBlock->process.type = type; // Tipo de proceso (normal, gráfico, etc.)
    newProcessBlock->process.state = PROCESS_STATE_NEW;     // Estado inicial del proceso
    newProcessBlock->process.priority = priority;
    newProcessBlock->parent = parent; // Guardar el padre del proceso, si es que tiene uno
    
    newProcessBlock->stackBase = allocateStack(processCount); // Asignar stack predefinido
    newProcessBlock->registers.rsp = newProcessBlock->stackBase - 8; // Inicializar stack pointer y restar lo que se va a usar para el ret a quitProgram
    newProcessBlock->registers.rdi = (uint64_t)arguments; // Guardar el argumento en los registros del proceso, el resto de los registros son basura


    if (newProcessBlock->stackBase == NULL) {
        log_to_serial("addProcessToScheduler: Error al alocar memoria para el stack del proceso");
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

    log_hex("addProcessToScheduler: process stack base: ", newProcessBlock->stackBase);
    log_hex("addProcessToScheduler: process initial rsp: ", newProcessBlock->registers.rsp);
    log_hex("addProcessToScheduler: process rip: ", cri.rip);
    log_decimal("addProcessToScheduler: process pid: ", newProcessBlock->process.pid);

    // cargar en el stack del proceso el cri generado, y actualizar el stack pointer para que apunte al interrupt stack frame
    newProcessBlock->registers.rsp = load_interrupt_frame(&cri, newProcessBlock->registers.rsp);

    log_hex("addProcessToScheduler: process rsp after loading cri: ", newProcessBlock->registers.rsp);

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
    log_to_serial("addProcessToScheduler: Proceso agregado con exito");
    return newProcessBlock; // Retornar el nuevo proceso agregado
}


Pid newProcess(Program program, char *arguments, Priority priority, Pid parent_pid) {
    // para debug
    if(parent_pid == 0) {
        log_to_serial("newProcess: Creando nuevo proceso sin padre, Init");
    } else {
        log_to_serial("newProcess: Creando nuevo proceso con padre");
    }

    if(parent_pid == 0 && processList != NULL) {
        log_to_serial("newProcess: Init ya existe, error!");
        return 0; // No se puede crear un proceso sin padre si ya hay un init
    }

    ProcessControlBlock * parent = getProcessControlBlock(parent_pid);
    if(parent == NULL  && processList != NULL){
        log_to_serial("invalid parent process");
        return NULL;
    }
    if(parent->process.type != PROCESS_TYPE_MAIN){
        parent = getProcessControlBlock(parent->parent);
    }

    ProcessControlBlock * newProcessBlock = addProcessToScheduler(program, program.entry, arguments, PROCESS_TYPE_MAIN, priority, parent);

    if (IS_GRAPHIC(newProcessBlock)) {
        uint8_t *buffer = addWindow(newProcessBlock->process.pid);
        if (buffer == NULL) {
            log_to_serial("addProcessToScheduler: Error al agregar la ventana del proceso grafico");
            return 0;
        }
    }

    log_to_serial("newMainProcess: Agregando nuevo proceso al scheduler");
    log_decimal("newMain with PID: ", newProcessBlock->process.pid);
    return newProcessBlock->process.pid;
}

Pid newThread(ProgramEntry entrypoint, char *arguments, Priority priority, Pid parent_pid) {
    // Tengo que validar que el parent sea un proceso main y no un thread sino tengo que crear el thread a nombre del main del thread

    ProcessControlBlock * parent = getProcessControlBlock(parent_pid);
    if(parent == NULL){
        log_to_serial("invalid parent process");
        return NULL;
    }
    if(parent->process.type != PROCESS_TYPE_MAIN){
        parent = getProcessControlBlock(parent->parent);
    }

    // le saco permisos gráficos al thread
    Program threadProgram = parent->process.program;
    // threadProgram.permissions &= ~DRAWING_PERMISSION; // Quitar permisos gráficos al thread
    ProcessControlBlock * newProcessBlock = addProcessToScheduler(threadProgram, entrypoint, arguments, PROCESS_TYPE_THREAD, priority, parent);
    
    log_to_serial("newThread: Agregando nuevo thread al scheduler:");
    log_decimal("newThread with PID: ", newProcessBlock->process.pid);
    return newProcessBlock->process.pid;
}


//! NO USAR DIRECTAMENTE, USAR terminateProcess QUE ES RECURSIVA, ESTO PODRÍA DEJAR PROCESOS HUÉRFANOS
//! Cuando se llame, se da por sentado que el proceso existe y puede matarse, las validaciones deben hacerse antes de llamar a esta función
int terminateSingleProcess(uint32_t pid) {

    ProcessControlBlock * to_remove = getProcessControlBlock(pid);
    if(to_remove == NULL){
        log_to_serial("invalid parent process");
        return -1;
    }
    if(to_remove->process.pid == 1 || to_remove->next == to_remove){
        // O estás borrando init o estás borrando el único proceso que hay, ilegalísimo
        return -2;
    }

    // Si el proceso es gráfico, eliminar la ventana asociada
    uint64_t was_graphic = IS_GRAPHIC(to_remove);
    if(was_graphic) {
        log_to_serial("terminateCurrentProcess: El proceso actual es grafico, eliminando ventana asociada");
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
        log_to_serial("invalid parent process");
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
    log_to_serial("scheduleNextProcess: Programando el siguiente proceso");

    if (currentProcessBlock == NULL) return;

    currentProcessBlock->process.state = currentProcessBlock->process.state == PROCESS_STATE_RUNNING ? PROCESS_STATE_READY : currentProcessBlock->process.state; // Cambiar el estado del proceso actual a READY

    ProcessControlBlock *nextProcess = currentProcessBlock->next; // Guardar el siguiente proceso antes de liberar el actual
    if(currentProcessBlock->process.state == PROCESS_STATE_TERMINATED) {
        log_to_serial("scheduleNextProcess: El proceso actual ya esta terminado, no se puede programar otro proceso");
        free(currentProcessBlock->stackBase); // Liberar el stack del proceso actual
        free(currentProcessBlock); // Liberar el PCB del proceso actual

    } 

    ProcessControlBlock * current = nextProcess;
    ProcessControlBlock * prev = currentProcessBlock; // Empezar desde el final de la lista para poder eliminar el proceso actual si es necesario
    currentProcessBlock = nextProcess; // Actualizar el proceso actual al siguiente

    // Pasa de largo los que no están en estado READY o NEW
    while (current->process.state != PROCESS_STATE_READY && current->process.state != PROCESS_STATE_NEW) {
        if(current->process.state == PROCESS_STATE_TERMINATED) {
            nextProcess = current->next; // Guardar el siguiente proceso antes de liberar el actual

            log_to_serial("scheduleNextProcess: El proceso actual ya esta terminado, no se puede programar otro proceso");
            free(current->stackBase); // Liberar el stack del proceso actual
            free(current); // Liberar el PCB del proceso actual

            prev->next = nextProcess; // Eliminar el proceso actual de la lista
            current = nextProcess;
            continue;
        } 

        // vuelvo a llegar, no había ningún proceso en estado READY o NEW
        if (current == currentProcessBlock) {
            log_to_serial("scheduleNextProcess: No hay procesos en estado READY");
            return; // No hay procesos en estado READY ni NEW, no se puede programar otro proceso
        }

        current = current->next;
        prev = prev->next; // Avanzar al siguiente proceso en la lista
    }
    currentProcessBlock = current; // Actualizar el proceso actual al siguiente que está en estado READY o NEW

    if (currentProcessBlock->process.state == PROCESS_STATE_NEW) {
        // Si el proceso es nuevo, inicializarlo
        currentProcessBlock->process.state = PROCESS_STATE_READY;
        log_to_serial("scheduleNextProcess: Proceso nuevo, inicializando");
    }

    currentProcessBlock->process.state = PROCESS_STATE_RUNNING; // Cambiar el estado del nuevo proceso a RUNNING

    // Actualizar el estado del kernel, indicando qué proceso está en ejecución, los permisos, y de-escalando los privilegios de kernel
    desactivateRootMode();

    // LOGS    
    log_string("scheduleNextProcess: Proceso actual:");
    log_string(currentProcessBlock->process.program.name);
    log_decimal("scheduleNextProcess: PID: ", currentProcessBlock->process.pid);
    log_string("scheduleNextProcess: Restaurando registros del proceso actual con magic_recover");

    log_hex("scheduleNextProcess: Stack base del proceso actual: ", currentProcessBlock->stackBase);
    log_hex("scheduleNextProcess: RSP del proceso actual: ", currentProcessBlock->registers.rsp);

    log_string(">>>>>>>>>>>>>>>> scheduleNextProcess: YENDO A MAGIC RECOVER");
    magic_recover(&currentProcessBlock->registers);
}


// Bucle del planificador, a ejecutar lo frecuentemente que se quiera, ej. cada timertick
void schedulerLoop() {
    // Si no hay procesos, no hacer nada

    ticksSinceLastSwitch++;
    if (processList == NULL || ticksSinceLastSwitch % TICKS_TILL_SWITCH != 0) {
        log_to_serial("schedulerLoop: nada que hacer");
        return;
    }

    log_to_serial("schedulerLoop: Ejecutando el bucle del scheduler");

    if(currentProcessBlock == NULL){
        currentProcessBlock = processList;
    } else {
        // Guardar el contexto del proceso actual en base al backup de registros, para poder restaurarlo después
        BackupRegisters * backup = getBackupRegisters();
        currentProcessBlock->registers = backup->registers;
    }

    log_to_serial("schedulerLoop: pasando al siguiente proceso");

    scheduleNextProcess();
}

// Ejecuta un callback recursivamente en todos los descendientes del proceso especificado
// No corre en el parent, solo en los descendientes
void runOnChilds(void (*callback)(ProcessControlBlock *), Pid parent_pid) {
    if (processList == NULL || callback == NULL) {
        log_to_serial("runOnChilds: Lista de procesos vacia o callback invalido");
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



// Deja el proceso en espera
int setWaiting(Pid pid)
{
    log_to_serial("setWaiting: Poniendo el proceso en espera");

    // if (currentProcessBlock == NULL) {
    //     log_to_serial("setWaiting: No hay proceso actual");
    //     return -1; // Error: no hay proceso actual
    // }

    ProcessControlBlock *processControlBlocks = processList;
    if (processControlBlocks == NULL) {
        log_to_serial("setWaiting: No hay procesos en la lista");
        return -1; // Error: no hay procesos en la lista
    }
    while (processControlBlocks != processListTail->next) {
        if (processControlBlocks->process.pid == pid) {
            // Cambiar el estado del proceso a WAITING
            processControlBlocks->process.state = PROCESS_STATE_WAITING;
            log_decimal("setWaiting: Proceso ", pid);
            log_to_serial("setWaiting: Proceso puesto en espera");
            scheduleNextProcess(); // Cambiar al siguiente proceso
            return 0; // Éxito
        }
        processControlBlocks = processControlBlocks->next;
    }

    log_to_serial("setWaiting: Proceso no encontrado");
    return -1; // Error: proceso no encontrado
}


 // Despierta un proceso que estaba en espera
int wakeProcess(Pid pid)
{
    log_to_serial("wakeProcess: Despertando el proceso");
    ProcessControlBlock *processControlBlocks = processList;
    if (processControlBlocks == NULL) {
        log_to_serial("wakeProcess: No hay procesos en la lista");
        return -1; // Error: no hay procesos en la lista
    }
    while (processControlBlocks != processListTail->next) {
        if (processControlBlocks->process.pid == pid) {
            // Cambiar el estado del proceso a READY
            processControlBlocks->process.state = PROCESS_STATE_READY;
            log_decimal("wakeProcess: Proceso ", pid);
            log_to_serial("wakeProcess: Proceso despertado");
            return 0; // Éxito
        }
        processControlBlocks = processControlBlocks->next;
    }
    log_to_serial("wakeProcess: Proceso no encontrado");
    return -1; // Error: proceso no encontrado
}