#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <processState.h>
#include <drivers/registersDriver.h>

#define IS_GRAPHIC(ProcessBlock) (ProcessBlock->process.program.permissions & DRAWING_PERMISSION)

typedef uint64_t Pid; // 64 bits para que puedas crear 40.000 procesos por segundo hasta que el sol muera sin tener que reiniciar el sistema

typedef enum {
    PRIORITY_LOW,
    PRIORITY_NORMAL,
    PRIORITY_HIGH,
} Priority;

typedef enum ProcessState {
    PROCESS_STATE_NEW,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_READY,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_TERMINATED,
} ProcessState;

typedef enum {
    PROCESS_TYPE_MAIN, // Proceso normal que ejecuta un programa
    PROCESS_TYPE_THREAD,  // Thread que comparte el mismo espacio de memoria que su padre
} ProcessType;


typedef struct Process {
    Pid pid;
    ProcessType type;
    ProcessState state;
    Priority priority;  // Prioridad del proceso, 0 es baja, 1 es normal, 2 es alta
    Program program;    // Programa asociado al proceso
} Process;


void initScheduler();
void schedulerLoop();

// Agrega un proceso al scheduler, devuelve el PID del proceso agregado o 0 si hubo un error
Pid newProcess(Program *program, char *arguments, Pid parent);

// Crea un nuevo hilo (thread) para un proceso existente, devuelve el PID del nuevo hilo o 0 si hubo un error
// La diferencia entre thread y proceso es que comparten el mismo espacio de memoria, no tienen una ventana asociada y no pueden tener hijos propios
// Si se crea un thread en otro thread, queda a nombre del proceso padre
Pid newThread(ProgramEntry entrypoint, char *arguments, Pid parent);

// Termina un proceso (o thread) dado, puede ser el actual o no, si es el actual no se va a volver de la función
// Si matás un proceso padre, mata a los hijos también (si hay algo que querés que no dependa del padre, ejecutalo como nohup)
// Retorna si fue exitoso o no
int terminateProcess(Pid pid);  

// Es necesario para saber cosas como en qué buffer de video escribir o darte cuenta de no matar el proceso actual
Pid getCurrentProcessPID();

// Pid 0 es inválido, por eso se devuelve un proceso con pid 0 si no hay proceso con el pid
// En ese caso program queda con basura
Process getProcess(Pid pid); // Devuelve el proceso del pid especificado

int setWaiting(Pid pid); // Deja el proceso en espera 
int wakeProcess(Pid pid); // Despierta un proceso que estaba en espera

// TODO --- acá para abajo ---

Pid * getAllProcesses(); // Devuelve una lista de todos los procesos en ejecución (para ps), pids null terminated

int changePriority(Pid pid, Priority newPriority); // Cambia la prioridad de un proceso, devuelve 0 si no se pudo cambiar, 1 si se cambió correctamente
Priority getPriority(Pid pid); // Obtiene la prioridad de un proceso, devuelve LOW, NORMAL o HIGH

// Pasa al siguiente proceso de la lista del scheduler
void scheduleNextProcess(); // es un yeld básicamente

// Guarda los registros del proceso actual en el backup de registros
// Importante ya que no solo se debe guardar el contexto en cada timertick, sino que debe guardarse en cada interrupción ya que puede no volver por donde vino
// Por ejemplo en una syscall que haga wait del proceso actual se necesita el contexto al que volvería la syscall para poder restaurarlo cuando esté ready
void backupCurrentProcessRegisters(); 

#endif

