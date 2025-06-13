#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <processManager/processState.h>
#include <drivers/registersDriver.h>
#include <types.h>

#define IS_GRAPHIC(ProcessBlock) (ProcessBlock->process.program.permissions & DRAWING_PERMISSION)


void initScheduler();
void schedulerLoop();

// Agrega un proceso al scheduler, devuelve el PID del proceso agregado o 0 si hubo un error
Pid newProcess(Program program, char *arguments, Priority priority, Pid parent_pid);

// Crea un nuevo hilo (thread) para un proceso existente, devuelve el PID del nuevo hilo o 0 si hubo un error
// La diferencia entre thread y proceso es que comparten el mismo espacio de memoria, no tienen una ventana asociada y no pueden tener hijos propios
// Si se crea un thread en otro thread, queda a nombre del proceso padre
Pid newThread(ProgramEntry entrypoint, char *arguments, Priority priority, Pid parent_pid);

// Termina un proceso (o thread) dado, puede ser el actual o no, si es el actual no se va a volver de la función
// Si matás un proceso padre, mata a los hijos también (si hay algo que querés que no dependa del padre, ejecutalo como nohup)
// Retorna si fue exitoso o no
int terminateProcess(Pid pid);  


// Pasa al siguiente proceso de la lista del scheduler
void scheduleNextProcess(); // es un yeld básicamente


// Es necesario para saber cosas como en qué buffer de video escribir o darte cuenta de no matar el proceso actual
Pid getCurrentProcessPID();

// Pid 0 es inválido, por eso se devuelve un proceso con pid 0 si no hay proceso con el pid
// En ese caso program queda con basura
Process getProcess(Pid pid); // Devuelve el proceso del pid especificado

// Devuelve el proceso padre del proceso especificado, si devuelve un proceso con pid 0, significa que no hay padre (es el init o no existe el proceso)
Process getParent(Pid pid);


// Retorna 1 si ambos procesos pertenecen al mismo grupo, 0 si no
// Un grupo de procesos es el proceso main y sus threads, ya que para muchas cosas se los considera como un solo proceso
// Por ejemplo, un grupo de procesos comparte stdin/stdout, la ventana gráfica, permisos sobre un archivo, etc.
int isSameProcessGroup(Pid pid1, Pid pid2);

// Devuelve el PID del proceso main del grupo al que pertenece el proceso especificado
Pid getProcessGroupMain(Pid pid);

// Devuelve si un proceso dado es descendiente de otro proceso (o sea, si es hijo, hijo de un hijo, thread de un hijo, etc.)
// Si el proceso es el mismo devuelve 1 (se toma como que un proceso siempre es descendiente de sí mismo)
int isDescendantOf(Pid child_pid, Pid parent_pid);

Process * getAllProcesses(); // Devuelve una lista de todos los procesos en ejecución (para ps), cuando se encuentre un proceso con pid 0, significa el final de la lista

int changePriority(Pid pid, Priority newPriority); // Cambia la prioridad de un proceso, devuelve -1 si no se pudo cambiar, 0 si se cambió correctamente
Priority getPriority(Pid pid); // Obtiene la prioridad de un proceso, devuelve LOW, NORMAL o HIGH


// --- Semáforos y esperas ---

int setWaiting(Pid pid); // Deja el proceso en espera, si es el actual no se vuelve de esta función ya que se pasa al siguiente proceso
int wakeProcess(Pid pid); // Despierta un proceso que estaba en espera

void sem_init(int initial_value); // inicializa un semáforo con el valor especificado, devuelve el id del semáforo creado
int sem_destroy(uint64_t id);     // destruye el semáforo con el id especificado, no se puede destruir si hay procesos esperándolo
void sem_wait(uint64_t id);       // decrementa el semáforo de id especificado, si el valor es menor a cero bloquea el proceso
void sem_post(uint64_t id);       // incrementa el semáforo de id especificado, si el valor es mayor o igual a cero despierta un proceso que estuviera esperando

// Guarda los registros del proceso actual en el backup de registros
// Importante ya que no solo se debe guardar el contexto en cada timertick, sino que debe guardarse en cada interrupción ya que puede no volver por donde vino
// Por ejemplo en una syscall que haga wait del proceso actual se necesita el contexto al que volvería la syscall para poder restaurarlo cuando esté ready
void backupCurrentProcessRegisters(); 

#endif

