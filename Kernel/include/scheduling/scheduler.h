#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <processState.h>
#include <drivers/registersDriver.h>

#define IS_GRAPHIC(currentProcess) (currentProcess->permissions & DRAWING_PERMISSION)

typedef uint64_t Pid; // 64 bits para que puedas crear 40.000 procesos por segundo hasta que el sol muera sin tener que reiniciar el sistema

typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH = 2,
} Priority;

typedef struct ProcessControlBlock {
    char *name;                     
    uint32_t pid;                   
    uint32_t permissions;           
    uint64_t stackBase;             // Dirección base del stack asignado, no es lo mismo que el rbp guardado ya que eso puede ser modificado por el proceso
    Registers registers;            // Registros del proceso, incluyendo el stack pointer (que en realidad apunta al interrupt stack frame, no al stack del proceso en sí)
    struct ProcessControlBlock *next; // Siguiente proceso (lista circular)
} ProcessControlBlock;

void initScheduler();
void schedulerLoop();

// Agrega un proceso al scheduler, devuelve el PID del proceso agregado o 0 si hubo un error
Pid newProcess(Program *program, char *arguments, int nohup);

// Crea un nuevo hilo (thread) para un proceso existente, devuelve el PID del nuevo hilo o 0 si hubo un error
// La diferencia entre thread y proceso es que comparten el mismo espacio de memoria, no tienen una ventana asociada y no pueden tener hijos propios
Pid newThread(ProgramEntry entrypoint, char *arguments, Pid parent);

// Termina un proceso dado, puede ser el actual o no, si es el actual no se va a volver de la función
// Si matás un proceso padre, mata a los hijos también (si hay algo que querés que no dependa del padre, ejecutalo como nohup)
// Retorna si fue exitoso o no
int terminateProcess(Pid pid);  

// Es necesario para saber cosas como en qué buffer de video escribir o darte cuenta de no matar el proceso actual
Pid getCurrentProcessPID();

// Hay que ver si tiene sentido devolver un PCB o si es mejor hacer otra estructura para representar procesos fuera del scheduler
ProcessControlBlock *getProcess(Pid pid); // Devuelve el PCB del proceso con el PID dado, o NULL si no existe


// TODO --- acá para abajo ---

getAllProcesses(); // Devuelve una lista de todos los procesos en ejecución (para ps)


int changePriority(Pid pid, Priority newPriority); // Cambia la prioridad de un proceso, devuelve 0 si no se pudo cambiar, 1 si se cambió correctamente
Priority getPriority(Pid pid); // Obtiene la prioridad de un proceso, devuelve LOW, NORMAL o HIGH

int setWaiting(Pid pid); // Deja el proceso en espera 
int wakeProcess(Pid pid); // Despierta un proceso que estaba en espera

// Pasa al siguiente proceso de la lista del scheduler
void scheduleNextProcess(); // es un yeld básicamente





#endif

