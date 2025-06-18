#include <drivers/serialDriver.h>
#include <processManager/scheduler.h>
#include <processManager/processState.h>
#include <stdint.h>
#include <stdlib.h>
#include <memoryManager/memoryManager.h>
#include <windowManager/windowManager.h>
#include <fileSystem/fileSystem.h>
#include <eventManager/eventManager.h>

#define STACK_SIZE 0x10000       // Tamaño de cada stack (4 KB)
#define TICKS_TILL_SWITCH 1     // Cantidad de ticks hasta cambiar de proceso

#define NULL 0
#define VALIDATE_IO_FILE(id) (!id || validateFileType(stdin, FILE_TYPE_FIFO))

// VALIDA QUE TODO EL STRUC FilePermissions sea != 0.
#define VALIDATE_FILE_PERMISSIONS(FilePermissions) \
    (FilePermissions.writing_owner != 0 && FilePermissions.reading_owner != 0 && \
     FilePermissions.writing_conditions != 0 && FilePermissions.reading_conditions != 0)
    

void printProcessList();

char * getPriorityText(Priority priority) {
    switch (priority) {
        case PRIORITY_LOW:
            return "LOW";
        case PRIORITY_NORMAL:
            return "NORMAL";
        case PRIORITY_HIGH:
            return "HIGH";
        default:
            return "UNKNOWN";
    }
}

char * getProcessStateText(ProcessState state) {
    switch (state) {
        case PROCESS_STATE_NEW:
            return "NEW";
        case PROCESS_STATE_READY:
            return "READY";
        case PROCESS_STATE_RUNNING:
            return "RUNNING";
        case PROCESS_STATE_WAITING:
            return "WAITING";
        case PROCESS_STATE_TERMINATED:
            return "TERMINATED";
        default:
            return "UNKNOWN";
    }
}

char * getProcessTypeText(ProcessType type) {
    switch (type) {
        case PROCESS_TYPE_MAIN:
            return "MAIN";
        case PROCESS_TYPE_THREAD:
            return "THREAD";
        default:
            return "UNKNOWN";
    }
}


extern void _hlt();
extern void _cli();
extern void _sti();


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
    uint64_t stdin;
    uint64_t stdout;                // Archivos de entrada/salida del proceso, deben mapearse al crear el proceso, si no se especifica no se asigna ninguno
    uint64_t stderr;
} ProcessControlBlock;


extern uint64_t load_interrupt_frame(InterruptStackFrame *frame, uint64_t stack_pointer);
extern void magic_recover(Registers *registers);
extern void magic_recover_old(InterruptStackFrame *cri, uint64_t args);
extern void push_to_custom_stack_pointer(uint64_t stack_pointer, uint64_t value_to_push);

static ProcessControlBlock *currentProcessBlock = NULL;  // Proceso actualmente en ejecución
static ProcessControlBlock *processList = NULL;     // Lista circular de procesos
static ProcessControlBlock *processListTail = NULL; 
static Pid nextPID = 1;             // Contador para asignar PIDs únicos
static uint64_t processCount = 0;   // Contador de procesos activos

static Semaphore *firstSemaphore = NULL;
static uint64_t nextSemaphoreId = 1; // Contador para asignar IDs únicos a los semáforos

static uint32_t ticksSinceLastSwitch = 0; // Contador de ticks desde el último cambio de proceso



void initScheduler() {
    // log_to_serial("initScheduler: Iniciando el scheduler");
    processList = NULL;
    currentProcessBlock = NULL;
    firstSemaphore = NULL; 
}




/// ----- GETTERS GENERALES ----- ///
/// ----------------------------- ///

// Mayor quantum significa que el proceso tiene más tiempo para ejecutarse
uint32_t getQuantumByPriority(Priority priority) {
    // Asignar quantum según la prioridad del proceso
    switch (priority) {
        case PRIORITY_LOW:
            return TICKS_TILL_SWITCH; 
        case PRIORITY_NORMAL:
            return TICKS_TILL_SWITCH * 2;
        case PRIORITY_HIGH:
            return TICKS_TILL_SWITCH * 3;
        default:
            return TICKS_TILL_SWITCH; // Por defecto, usar el quantum normal
    }
}

//! internal
static int log_process_search = 0;
ProcessControlBlock * getProcessControlBlock(Pid pid){
    if(processList == NULL) return NULL;
    // log_decimal(" #### getProcessControlBlock: Buscando proceso con PID: ", pid);
    int iterations = 0;
    // log_decimal("W: >>>>>>>>>> getProcessControlBlock: Buscando proceso con PID: ", pid); 
    // printProcessList(); // Imprimir la lista de procesos para depuración
    ProcessControlBlock * current = processList;
    do {
        if(log_process_search){
            // log_decimal("----------> getProcessControlBlock: analizando: ", current->process.pid);
            // log_decimal("----------> getProcessControlBlock: pid buscado: ", pid);
            // log_decimal("----------> getProcessControlBlock: current->next->process.pid: ", current->next->process.pid);
            // log_decimal("----------> getProcessControlBlock: iteraciones: ", iterations);
            // sleep(250);
        }

        if(current->process.pid == pid){
            log_process_search = 0; // Resetear el log de búsqueda
            return current; // Devuelve el proceso actual
        }
        current = current->next;
        iterations++;
    } while (current != processList);
    
    log_process_search = 0; // Resetear el log de búsqueda

    // log_to_serial("E: getProcessControlBlock: Proceso no encontrado");
    // console_log("E: getProcessControlBlock: Proceso con PID %d no encontrado tras %d iteraciones", pid, iterations);
    return NULL; 
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
            return current->process; // Devuelve el proceso actual
        }
        current = current->next;
    } while (current != processList);

    return invalid_process;
}

Process getParent(Pid pid){
    ProcessControlBlock * process = getProcessControlBlock(pid);
    if(process == NULL || process->parent == NULL) {
        log_to_serial("E: getParent: Proceso no encontrado o no tiene padre");
        return (Process){.pid = 0}; // No hay padre, devuelve un proceso inválido
    }
    return process->parent->process; // Devuelve el proceso padre
}

// si no existe el proceso devuelve todo 0, pero debería validarse antes ya que esos pueden ser I/O válidos
IO_Files getIOFiles(Pid pid){
    IO_Files IO_files = {0};

    ProcessControlBlock * process = getProcessControlBlock(pid);
    if(process == NULL){
        log_to_serial("E: getIOFiles: Proceso no encontrado");
        return IO_files;
    }

    IO_files.stdin = process->stdin;
    IO_files.stdout = process->stdout;
    IO_files.stderr = process->stderr;
    return IO_files;
}

// Devuelve una lista de todos los procesos en ejecución (para ps)
// Deja en count la cantidad de procesos encontrados
// Es tarea de quien llame a esta función liberar la memoria de la lista devuelta
Process * getAllProcesses(int *count) {
    if(count == NULL) return NULL;
    if(processList == NULL){
        *count = 0;
        return NULL;
    } 
    *count = processCount; 

    Process * processes = (Process *)malloc(sizeof(Process) * processCount);
    if(processes == NULL) return NULL; // Error al alocar memoria

    ProcessControlBlock * current = processList;
    uint32_t index = 0;

    do {
        processes[index++] = current->process;
        current = current->next;
    } while (current != processList);

    return processes;
}


int changePriority(Pid pid, Priority newPriority){ 
    ProcessControlBlock *current = getProcessControlBlock(pid);
    if (current == NULL) {
        log_to_serial("E: changePriority: Proceso no encontrado");
        return -1; // Error: proceso no encontrado
    }
    if (newPriority < PRIORITY_LOW || newPriority > PRIORITY_HIGH) {
        return -1; // Prioridad inválida
    }

    // log_to_serial("changePriority: Cambiando prioridad del proceso");
    current->process.priority = newPriority;
    current->quantum = getQuantumByPriority(newPriority); // Actualizar el quantum del proceso según la nueva prioridad

    // log_to_serial("changePriority: Prioridad cambiada con exito");
    // log_decimal("changePriority: Proceso con PID ", current->process.pid);
    return 0;
}

Priority getPriority(Pid pid) {
    ProcessControlBlock *current = getProcessControlBlock(pid);
    if (current == NULL) {
        log_to_serial("E: getPriority: Proceso no encontrado");
        return -1; 
    }
    return current->process.priority;
}


// Retorna 1 si ambos procesos pertenecen al mismo grupo, 0 si no
// Un grupo de procesos es el proceso main y sus threads, ya que para muchas cosas se los considera como un solo proceso
// Por ejemplo, un grupo de procesos comparte stdin/stdout, la ventana gráfica, permisos sobre un archivo, etc.
int isSameProcessGroup(Pid pid1, Pid pid2){
    ProcessControlBlock *pcb1 = getProcessControlBlock(pid1);
    ProcessControlBlock *pcb2 = getProcessControlBlock(pid2);

    if (pcb1 == NULL || pcb2 == NULL) {
        log_to_serial("E: isSameProcessGroup: Uno de los procesos no existe");
        return 0; // Uno de los procesos no existe
    }

    // Si alguno de los procesos es un thread, buscar su proceso main, luego simplemente comparar que sean del mismo PID
    pcb1 = pcb1->process.type == PROCESS_TYPE_THREAD ? pcb1->parent : pcb1; 
    pcb2 = pcb2->process.type == PROCESS_TYPE_THREAD ? pcb2->parent : pcb2; 

    return pcb1->process.pid == pcb2->process.pid; // Comparar los PIDs de los procesos main
}

// Devuelve el PID del proceso main del grupo al que pertenece el proceso especificado
Pid getProcessGroupMain(Pid pid) {
    ProcessControlBlock *pcb = getProcessControlBlock(pid);
    if (pcb == NULL) {
        log_to_serial("E: getProcessGroupMain: Proceso no encontrado");
        return 0; // Proceso no encontrado
    }

    return pcb->process.type == PROCESS_TYPE_THREAD ? pcb->parent->process.pid : pcb->process.pid; // Si es un thread, devolver el PID del proceso main, si no, devolver el PID del mismo proceso
}

// Devuelve si un proceso dado es descendiente de otro proceso (o sea, si es hijo, hijo de un hijo, thread de un hijo, etc.)
// Si el proceso es el mismo devuelve 1 (se toma como que un proceso siempre es descendiente de sí mismo)
int isDescendantOf(Pid child_pid, Pid parent_pid) {
    if(child_pid == 0 || parent_pid == 0)
        return 0; // PID 0 es inválido, no puede ser padre ni hijo

    if (child_pid == parent_pid)
        return 1; // Un proceso es descendiente de sí mismo

    ProcessControlBlock *current = getProcessControlBlock(child_pid);
    while (current != NULL && current->parent != NULL) {
        if (current->parent->process.pid == parent_pid) {
            return 1; // Encontramos el padre
        }
        current = current->parent; // Subir en la jerarquía de procesos
    }
    log_to_serial("E: isDescendantOf: Proceso hijo no encontrado");
    return 0; // No es descendiente
}



/// ----- FUNCIONES DE ALLOCACIÓN DE MEMORIA ----- ///
/// ---------------------------------------------- ///


ProcessControlBlock *allocateProcessMemory(size_t size) {
    return (ProcessControlBlock *)malloc(sizeof(ProcessControlBlock));
}

uint64_t allocateStack() {
    // Allocar STACK_SIZE bytes usando malloc
    void* stackMemory = malloc(STACK_SIZE);
    if (stackMemory == NULL) {
        return 0; // Error: no se pudo allocar memoria para el stack
    }

    
    // El stack crece hacia abajo, así que el stack base debe ser el final de la memoria allocada
    return (uint64_t)stackMemory + STACK_SIZE;
}


/// ----- HELPERS ------ ///
/// -------------------- ///

// Ejecuta un callback recursivamente en todos los descendientes del proceso especificado
// No corre en el parent, solo en los descendientes
void runOnChilds(void (*callback)(ProcessControlBlock *), Pid parent_pid) {
    if (processList == NULL || callback == NULL) {
        // log_to_serial("runOnChilds: Lista de procesos vacia o callback invalido");
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


// Esta función se llama cuando un proceso termina su ejecución normalmente
// La idea era inyectar una syscall que haga eso en la última línea de cada programa, pero por ahora no soportamos ELF, así que por ahora se le pone como ret (es una medida temporal)
extern void quitWrapper();
// {
    // _cli();
    // if(are_interrupts_enabled()) {
    //     console_log("E: !!!!!!!!!!!!!!!! quitWrapper: Interrupts enabled, disabling them before quitting");
    //     _cli(); // Deshabilitar interrupciones para evitar problemas al terminar el proceso
    // }
    // console_log("W: quitWrapper: Programa saliendo naturalmente. PID: %d", getCurrentProcessPID());
    // terminateProcess(getCurrentProcessPID()); // Terminar el proceso actual
//}



/// ----- GENERAL DE SHCEDULING (LOOP, SWITCH, CREAR Y DESTRUIR PROCESOS, ETC) ----- ///
/// -------------------------------------------------------------------------------- ///

// -- Crear procesos -- //

// Agrega un nuevo proceso al planificador (función interna); no lo ejecuta inmediatamente
// Le asigna un PID, inicializa el stack y los registros, y lo agrega a la lista de procesos
// No se encarga de crear la ventana ni asignar I/O, eso lo debe hacer quien llame a esta función
ProcessControlBlock * addProcessToScheduler(Program program, ProgramEntry entry, char *arguments, ProcessType type, Priority priority, ProcessControlBlock *parent) {

    // log_to_serial("addProcessToScheduler: Iniciando la creacion de un nuevo proceso");

    if (program.entry == NULL, entry == NULL) {
        log_to_serial("E: addProcessToScheduler: Error, invalid input");
        return NULL;
    }
    // log_to_serial("addProcessToScheduler: Agregando proceso");

    if( parent == NULL && processList != NULL) {
        log_to_serial("E: addProcessToScheduler: Init ya existe, error!");
        return NULL; // No se puede crear un proceso sin padre si ya hay un init
    }
        

    // Allocar memoria para el PCB usando malloc
    // Se guardan los datos del programa, se asigna un pid, y se inicializan los punteros
    // El stack se alloca dinámicamente usando malloc
    ProcessControlBlock *newProcessBlock = (ProcessControlBlock *)allocateProcessMemory(sizeof(ProcessControlBlock));

    if (newProcessBlock == NULL) {
        log_to_serial("E: addProcessToScheduler: Error al alocar memoria para el PCB");
        return NULL; // Error al alocar memoria
    }

    newProcessBlock->process.program = program; // Guardar los datos del programa
    newProcessBlock->process.pid = nextPID++;
    // log_to_serial("addProcessToScheduler : Asignando PID al nuevo proceso");
    // log_decimal(">>>>>>>>>>>>>>>>>>>>>>>>>>>> . addProcessToScheduler: PID asignado: ", newProcessBlock->process.pid);

    newProcessBlock->process.type = type;
    newProcessBlock->process.state = PROCESS_STATE_NEW;         // Estado inicial del proceso
    newProcessBlock->process.priority = priority;
    newProcessBlock->parent = parent;                           // Guardar el padre del proceso, si es que tiene uno
    newProcessBlock->quantum = getQuantumByPriority(priority);  // Cantidad de ticks que el proceso puede ejecutar antes de ser interrumpido
    newProcessBlock->waiting_for = NULL;                        // Inicializar el semáforo de espera como NULL

    // Los descriptores de I/O en principio no se asignan, quien llame a esta función debe encargarse de asignarlos si es necesario
    newProcessBlock->stdin = 0;
    newProcessBlock->stdout = 0;
    newProcessBlock->stderr = 0;
    
    newProcessBlock->stackBase = allocateStack();
    char stackStr[MEDIUM_TEXT_SIZE];
    strcpy(stackStr, "Process Stack: ");
    strcpy(stackStr + strlen(stackStr), newProcessBlock->process.program.name);

    // mem_register_sector(newProcessBlock->stackBase - STACK_SIZE, newProcessBlock->stackBase, stackStr);

    if (newProcessBlock->stackBase == NULL) {
        log_to_serial("E: addProcessToScheduler: Error al alocar memoria para el stack del proceso");
        free(newProcessBlock); // Liberar el PCB si no se pudo alocar el stack
        return NULL; // Error al alocar memoria
    }

    newProcessBlock->registers.rsp = newProcessBlock->stackBase - 8;    // Inicializar stack pointer y restar lo que se va a usar para el ret a quitProgram
    newProcessBlock->registers.rdi = (uint64_t)arguments;               // Guardar el argumento en los registros del proceso, el resto de los registros son basura

    // Guardo un puntero a la función de salida del programa, que es quitProgram (por eso rsp - 8)
    push_to_custom_stack_pointer(newProcessBlock->stackBase, (uint64_t)quitWrapper);

    // Generar el CRI inicial para el proceso
    // El rip se inicializa al entry point del programa, y el rsp al stack pointer del proceso antes de cargar el interrupt stack frame
    InterruptStackFrame cri = getDefaultCRI();
    cri.rip = (uint64_t) entry; // Entry point del programa
    cri.rsp = newProcessBlock->registers.rsp; // Usar el stack pointer del proceso

    // log_hex("addProcessToScheduler: process stack base: ", newProcessBlock->stackBase);
    // log_hex("addProcessToScheduler: process initial rsp: ", newProcessBlock->registers.rsp);
    // log_hex("addProcessToScheduler: process rip: ", cri.rip);
    // log_decimal("addProcessToScheduler: process pid: ", newProcessBlock->process.pid);

    // cargar en el stack del proceso el cri generado, y actualizar el stack pointer para que apunte al interrupt stack frame
    newProcessBlock->registers.rsp = load_interrupt_frame(&cri, newProcessBlock->registers.rsp);

    // log_hex("addProcessToScheduler: process rsp after loading cri: ", newProcessBlock->registers.rsp);

    // Los procesos son listas circulares, round robin básico
    // Si es el primer proceso, inicializar la lista (solo pasaría con el init)
    if (processList == NULL) {
        processList = newProcessBlock;
    }
    processListTail->next = newProcessBlock;  
    newProcessBlock->next = processList;      
    processListTail = newProcessBlock;

    processCount++;
    // log_to_serial("addProcessToScheduler: Proceso agregado con exito");

    // printProcessList(); // Para debug, imprimir la lista de procesos
    return newProcessBlock; // Retornar el nuevo proceso agregado
}


Pid newProcessWithIO(Program program, char *arguments, Priority priority, Pid parent_pid, uint64_t stdin, uint64_t stdout, uint64_t stderr) {
    // para debug
    if(parent_pid == 0) {
        // log_to_serial("newProcess: Creando nuevo proceso sin padre, Init");
    } else {
        // log_to_serial("newProcess: Creando nuevo proceso con padre");
    }

    if(!VALIDATE_IO_FILE(stdin) || !VALIDATE_IO_FILE(stdout) || !VALIDATE_IO_FILE(stderr)) {
        log_to_serial("E: newProcess: algun descriptor no es un FIFO valido");
        return 0;
    }

    if(parent_pid == 0 && processList != NULL) {
        log_to_serial("E: newProcess: Init ya existe, error!");
        return 0; // No se puede crear un proceso sin padre si ya hay un init
    }

    ProcessControlBlock * parent = getProcessControlBlock(parent_pid);
    if(parent == NULL  && processList != NULL){
        log_to_serial("E: invalid parent process");
        return NULL;
    }
    if(parent->process.type != PROCESS_TYPE_MAIN){
        parent = parent->parent; // Si el padre no es un proceso main, buscar al padre main (basicamente, si es un thread, buscar al main del thread)
        if(parent == NULL) {
            log_to_serial("E: newProcess: invalid parent process, no main parent found");
            return 0; // No se encontró un padre main válido
        }
    }

    ProcessControlBlock * newProcessBlock = addProcessToScheduler(program, program.entry, arguments, PROCESS_TYPE_MAIN, priority, parent);
    if (newProcessBlock == NULL) {
        log_to_serial("E: addProcessToScheduler: Error al agregar el proceso al scheduler");
        return 0; 
    }
    
    
    // Asignar los descriptores de I/O del proceso
    // --------------------------------------------
    if(stdin){
        // log_to_serial("W: newProcess: Asignando stdin al proceso");
        // log_decimal("newProcess: stdin file id: ", stdin);
        
        FilePermissions stdin_permissions = getFilePermissions(stdin); 
        if (!VALIDATE_FILE_PERMISSIONS(stdin_permissions)) {
            log_to_serial("E: newProcess: stdin permissions invalidos");
            terminateProcess(newProcessBlock->process.pid); // Terminar el proceso si los permisos son inválidos
            return 0; // Error: permisos inválidos
        }
        stdin_permissions.reading_owner = newProcessBlock->process.pid;
        stdin_permissions.reading_conditions = '.';
        
        // log_to_serial("newProcess: stdin permissions set");
        // log_decimal("newProcess: stdin permissions reading owner: ", stdin_permissions.reading_owner);
        // log_decimal("newProcess: stdin permissions reading conditions: ", stdin_permissions.reading_conditions);
        // log_decimal("newProcess: stdin permissions writing owner: ", stdin_permissions.writing_owner);
        // log_decimal("newProcess: stdin permissions writing conditions: ", stdin_permissions.writing_conditions);
        
        setFilePermissions(stdin, 0, stdin_permissions);
        newProcessBlock->stdin = stdin;
        
        log_to_serial("I: newProcess: stdin asignado al proceso");
    }
    if(stdout){
        // log_to_serial("W: newProcess: Asignando stdout al proceso");
        // log_decimal("newProcess: stdout file id: ", stdout);

        FilePermissions stdout_permissions = getFilePermissions(stdout);
        if (!VALIDATE_FILE_PERMISSIONS(stdout_permissions)) {
            log_to_serial("E: newProcess: stdout permissions invalidos");
            terminateProcess(newProcessBlock->process.pid); // Terminar el proceso si los permisos son inválidos
            return 0; // Error: permisos inválidos
        }
        stdout_permissions.writing_owner = newProcessBlock->process.pid;
        stdout_permissions.writing_conditions = '.';

        
        setFilePermissions(stdout, 0, stdout_permissions);
        newProcessBlock->stdout = stdout;
        
        log_to_serial("I: newProcess: stdout asignado al proceso");
        log_to_serial("newProcess: stdout permissions set");
        log_decimal("newProcess: stdout permissions reading owner: ", stdout_permissions.reading_owner);
        log_decimal("newProcess: stdout permissions reading conditions: ", stdout_permissions.reading_conditions);
        log_decimal("newProcess: stdout permissions writing owner: ", stdout_permissions.writing_owner);
        log_decimal("newProcess: stdout permissions writing conditions: ", stdout_permissions.writing_conditions);
    }
    if(stderr){
        // log_to_serial("W: newProcess: Asignando stderr al proceso");
        // log_decimal("newProcess: stderr file id: ", stderr);

        FilePermissions stderr_permissions = getFilePermissions(stderr);
        if (!VALIDATE_FILE_PERMISSIONS(stderr_permissions)) {
            log_to_serial("E: newProcess: stderr permissions invalidos");
            terminateProcess(newProcessBlock->process.pid); // Terminar el proceso si los permisos son inválidos
            return 0; // Error: permisos inválidos
        }
        stderr_permissions.writing_owner = newProcessBlock->process.pid;
        stderr_permissions.writing_conditions = '.';
        
        setFilePermissions(stderr, 0, stderr_permissions);
        newProcessBlock->stderr = stderr;
        
        log_to_serial("I: newProcess: stderr asignado al proceso");
        log_to_serial("newProcess: stderr permissions set");
        log_decimal("newProcess: stderr permissions reading owner: ", stderr_permissions.reading_owner);
        log_decimal("newProcess: stderr permissions reading conditions: ", stderr_permissions.reading_conditions);
        log_decimal("newProcess: stderr permissions writing owner: ", stderr_permissions.writing_owner);
        log_decimal("newProcess: stderr permissions writing conditions: ", stderr_permissions.writing_conditions);
    }

    if (stdin || stdout || stderr) {
        log_to_serial("newProcess: Descriptores de I/O asignados correctamente");
    } else {
        // log_to_serial("newProcess: No se asignaron descriptores de I/O, el proceso no podrá leer ni escribir");
    }

    // Si es gráfico, crear la ventana asociada
    if (newProcessBlock->process.program.permissions & DRAWING_PERMISSION) {
        // log_to_serial("newProcess: El proceso es grafico, creando ventana asociada");
        uint8_t *buffer = addWindow(newProcessBlock->process.pid);
        if (buffer == NULL) {
            // console_log("E: newProcess: No se pudo crear la ventana para el proceso con PID %d", newProcessBlock->process.pid);
            terminateProcess(newProcessBlock->process.pid); // Si no se pudo crear la ventana, eliminar el proceso (capaz es demasiado drástico, no sé, para pensar)
        }
    }

    // log_to_serial("newMainProcess: Agregando nuevo proceso al scheduler");
    // log_decimal("newMain with PID: ", newProcessBlock->process.pid);
    return newProcessBlock->process.pid;
}

Pid newProcess(Program program, char *arguments, Priority priority, Pid parent_pid){
    return newProcessWithIO(program, arguments, priority, parent_pid, 0, 0, 0); // Llamar a la función con descriptores de I/O nulos (nomás por backwards compatibility) 
}

// Si se crea un thread desde un thread, se le asigna de padre el main del thread que lo creó
Pid newThread(ProgramEntry entrypoint, char *arguments, Priority priority, Pid parent_pid) {
    // Tengo que validar que el parent sea un proceso main y no un thread sino tengo que crear el thread a nombre del main del thread

    ProcessControlBlock * parent = getProcessControlBlock(parent_pid);
    if(parent == NULL){
        // console_log("E: newThread: Proceso padre no encontrado (PID %d), no se puede crear el thread", parent_pid); 
        return NULL;
    }
    if(parent->process.type != PROCESS_TYPE_MAIN){
        parent = parent->parent;
    }

    ProcessControlBlock * newProcessBlock = addProcessToScheduler(parent->process.program, entrypoint, arguments, PROCESS_TYPE_THREAD, priority, parent);
    if (newProcessBlock == NULL) {
        log_to_serial("E: addProcessToScheduler: Error al agregar el thread al scheduler");
        return 0;
    }

    // Asignar los descriptores de I/O del thread (hereda los del padre así que no hay que setear permisos ni validar nada)
    newProcessBlock->stdin = parent->stdin;
    newProcessBlock->stdout = parent->stdout;
    newProcessBlock->stderr = parent->stderr;
    
    // log_to_serial("W: newThread: Agregando nuevo thread al scheduler:");
    // log_decimal("I: newThread with PID: ", newProcessBlock->process.pid);
    return newProcessBlock->process.pid;
}


// -- Matar procesos -- //

//! NO USAR DIRECTAMENTE, USAR terminateProcess QUE ES RECURSIVA, ESTO PODRÍA DEJAR PROCESOS HUÉRFANOS
//! Cuando se llame, se da por sentado que el proceso existe y puede matarse, las validaciones deben hacerse antes de llamar a esta función
int terminateSingleProcess(uint32_t pid) {

    ProcessControlBlock * to_remove = getProcessControlBlock(pid);
    if(to_remove == NULL){
        log_to_serial("E: terminateSingleProcess: Proceso no encontrado");
        return -1;
    }
    if(to_remove->process.pid == 1 || to_remove->next == to_remove){
        // O estás borrando init o estás borrando el único proceso que hay, ilegalísimo
        return -2;
    }

    // Si el proceso es gráfico, eliminar la ventana asociada
    uint64_t was_graphic = IS_GRAPHIC(to_remove);
    if(was_graphic) {
        // log_to_serial("terminateCurrentProcess: El proceso actual es grafico, eliminando ventana asociada");
        removeWindow(pid); 
    }

    // Marcarlo como terminado para que el scheduler lo elimine
    to_remove->process.state = PROCESS_STATE_TERMINATED;
    processCount--;

    // Cerrar la escritura de todos los fifos del proceso
    closeAllFifosOfProcess(to_remove->process.pid);

    // console_log("W: terminateSingleProcess: Proceso con PID %d terminado", to_remove->process.pid);
    // Emitir evento de muerte
    handleProcessDeath(to_remove->process.pid); 
}


// Matar un proceso borra su ventana, avisa que murió con un evento, y lo marca como terminado para que el bucle del scheduler lo elimine
// Es recursivo para así matar también a los hijos
int terminateProcess(Pid pid) {
    // log_to_serial("E: terminateProcess: Terminando proceso");
    // printProcessList(); // Para debug, imprimir la lista de procesos antes de eliminar el proceso
    ProcessControlBlock * to_remove = getProcessControlBlock(pid);
    if(to_remove == NULL){
        log_to_serial("E: terminateProcess: Proceso no encontrado");
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
    // console_log("E: xd");
    return 0; // Proceso eliminado exitosamente
}


// -- Schedule next y scheduler loop -- //

void scheduleNextProcess() {
    // log_to_serial("I: scheduleNextProcess: Programando el siguiente proceso");

    if (currentProcessBlock == NULL) return;

    currentProcessBlock->process.state = currentProcessBlock->process.state == PROCESS_STATE_RUNNING ? PROCESS_STATE_READY : currentProcessBlock->process.state; // Cambiar el estado del proceso actual a READY

    ProcessControlBlock *nextProcess = currentProcessBlock->next;
    ProcessControlBlock * current = nextProcess;
    ProcessControlBlock * prev = currentProcessBlock; // Empezar desde el final de la lista para poder eliminar el proceso actual si es necesario
    // currentProcessBlock = nextProcess; // Actualizar el proceso actual al siguiente

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
            // mem_free_sector(current->stackBase - STACK_SIZE); // Liberar el sector de memoria del stack del proceso actual
            free(current); // Liberar el PCB del proceso actual

            prev->next = nextProcess; // Eliminar el proceso actual de la lista
            current = nextProcess;
            continue;
        }
        if (current->process.state == PROCESS_STATE_WAITING) {
            // log_to_serial("W: scheduleNextProcess: Proceso en estado WAITING, saltando");
        }

        // vuelvo a llegar, no había ningún proceso en estado READY o NEW
        if (current == currentProcessBlock) {
            log_to_serial("E: scheduleNextProcess: No hay procesos en estado READY");
            // No hay procesos en estado READY ni NEW, no se puede programar otro proceso

            // Imprimir la dirección del stack actual para debug (NO DEL PROCESO, SINO EL STACK ACTUAL, USAR ASSEMBLER SI ES NECESARIO)
            // uint64_t stackPointer;
            // __asm__ __volatile__("mov %%rsp, %0" : "=r"(stackPointer));
            // mem_log_address(stackPointer, "scheduleNextProcess: Stack base del proceso actual");
            _hlt();
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
    // log_decimal("I: scheduleNextProcess: PID: ", currentProcessBlock->process.pid);
    // log_to_serial("scheduleNextProcess: Restaurando registros del proceso actual con magic_recover");

    // log_hex("scheduleNextProcess: Stack base del proceso actual: ", currentProcessBlock->stackBase);
    // log_hex("scheduleNextProcess: RSP del proceso actual: ", currentProcessBlock->registers.rsp);

    // log_to_serial(">>>>>>>>>>>>>>>> scheduleNextProcess: YENDO A MAGIC RECOVER");
    ticksSinceLastSwitch = 0; // Reiniciar el contador de ticks desde el último cambio de proceso

    // log_decimal("I: scheduleNextProcess: Proceso con PID ", currentProcessBlock->process.pid);
    magic_recover(&currentProcessBlock->registers);
}


// Bucle del planificador, a ejecutar lo frecuentemente que se quiera, ej. cada timertick
// Bucle del planificador, a ejecutar lo frecuentemente que se quiera, ej. cada timertick
void schedulerLoop() {

    // Solo schedulea si hay procesos y pasó el quantum del proceso
    ticksSinceLastSwitch++;
    if (processList == NULL || (currentProcessBlock != NULL && ticksSinceLastSwitch < currentProcessBlock->quantum))
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




/// ------ WAIT Y AWAKE ------ ///
/// -------------------------- ///


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
                // log_to_serial("I: setWaiting: Proceso actual puesto en espera");
                // Si el proceso actual es el que se está poniendo en espera, programar el siguiente proceso
                scheduleNextProcess(); // Cambiar al siguiente proceso
            }

            return 0; // Éxito
        }
        current = current->next;
    }
    while (current != processList);

    log_to_serial("E: setWaiting: Proceso no encontrado");
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
            // Cambiar el estado del proceso a READY
            current->process.state = PROCESS_STATE_READY;
            current->waiting_for = NULL; // Si estaba esperando un semáforo, limpiarlo
            return 0; // Éxito
        }
        current = current->next;
    }
    while (current != processList);

    return -1; // Error: proceso no encontrado
}



/// ----- SEMÁFOROS ----- ///


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


// TODO: que los semáforos tengan permisos, al igual que los archivos, syscalls, etc. 
//     -> (Básicamente quién puede modificarlo/esperarlo, si el proceso actual y sus threads, el actual e hijos, todas las instancias del programa, o todos.)
uint64_t sem_init(int initial_value) {
    Semaphore* sem = (Semaphore*)malloc(sizeof(Semaphore));
    if(sem == NULL) return 0;

    sem->id = nextSemaphoreId++; 
    sem->value = initial_value;
    sem->next = firstSemaphore;
    firstSemaphore = sem; 

    return sem->id;
}


int sem_destroy(uint64_t id) {
    Semaphore *sem = getSemaphore(id);
    if (sem == NULL) {
        return 1; // No se encontró el semáforo, no hay nada que destruir
    }

    // validar que nadie esté esperando este semáforo
    ProcessControlBlock *current = processList;
    do {
        if (current->waiting_for == sem) {
            // Hay un proceso esperando este semáforo, no se puede destruir
            return -1; 
        }
        current = current->next;
    } while (current != processList);

    // Buscar el semáforo en la lista y eliminarlo
    Semaphore *currentSem = firstSemaphore;
    Semaphore *prev = NULL;
    while (currentSem != NULL) {
        if (currentSem->id == id) {
            if (prev == NULL) {
                firstSemaphore = currentSem->next; // Actualizar la cabeza de la lista si es el primero
            } else {
                prev->next = currentSem->next; // Eliminar el semáforo de la lista
            }
            free(currentSem); // Liberar la memoria del semáforo
            return 0; 
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
        currentProcessBlock->waiting_for = sem; // Guardar el semáforo que está esperando
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


// FOR DEBUGGING PURPOSES ONLY
void printProcessList() {
    ProcessControlBlock *current = processList;
    if (current == NULL) {
        log_to_serial("No hay procesos en la lista");
        return;
    }

    log_to_serial(">>>>>>>>>>>>> Lista de procesos:");
    do {
        // console_log("PID: %d, Name: %s, Type: %s, State: %s, Priority: %s, Parent PID: %d",
        //     current->process.pid,
        //     current->process.program.name,
        //     getProcessTypeText(current->process.type),
        //     getProcessStateText(current->process.state),
        //     getPriorityText(current->process.priority),
        //     current->parent ? current->parent->process.pid : -1
        // );
        log_decimal("PID: ", current->process.pid);
        log_to_serial(current->process.program.name);
        log_to_serial(getProcessTypeText(current->process.type));
        log_to_serial(getProcessStateText(current->process.state));
        log_to_serial(getPriorityText(current->process.priority));
        log_decimal("Parent PID: ", current->parent ? current->parent->process.pid : -1);
        log_to_serial("================================");
        current = current->next;
    } while (current != processList);
    log_to_serial("<<<<<<<<<<<<< Fin de la lista de procesos");
}