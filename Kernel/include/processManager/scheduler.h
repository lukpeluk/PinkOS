#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <processManager/processState.h>
#include <drivers/registersDriver.h>
#include <types.h>

#define IS_GRAPHIC(ProcessBlock) (ProcessBlock->process.program.permissions & DRAWING_PERMISSION)

/* DOCS
    * Términos/conceptos del manejo de procesos en PinkOS:
    * ----------------------------------------------------
        * Hay procesos y threads;
        *   -> Aunque a todo le llamamos proceso, así que decimos que hay dos tipos de procesos: main y thread.
            * Un proceso main puede tener hijos propios, una ventana asociada en el caso de ser gráfico, y puede crear múltiples threads.
            * Cuando soportemos archivos binarios, un proceso main será el que tenga su propio espacio de memoria (que compartirá con sus threads)
            * Un thread es un proceso que comparte varias cosas con su proceso padre, como el stdin/stdout, permisos, ventana asociada, etc.
            * Un thread no puede tener hijos ni threads propios; se creará todo a nombre del proceso main del thread.
            * Puede pensarse a un thread como un worker del padre, se usa para hacer tareas en paralelo a nombre del mismo proceso, y para manejar los eventos asincrónicamente.
            * Un proceso se crea a partir de un programa, un thread a partir de una función dentro del programa
            * Cuando un proceso main muere, se matan a todos sus hijos incluyendo a sus threads
            * Cuando un thread muere, no pasa nada ya que no tiene hijos propios y todos sus archivos también son del padre así que ni deben cerrarse
            * Ambos procesos y threads tienen PID, y a efectos de scheduling (prioridad, semáforos, quantum, etc.) son tratados igual
            
        * Le llamamos "grupo de procesos" a un proceso main y sus threads
            * Este es un concepto útil ya que comparten muchas cosas, como stdin/stdout, permisos sobre archivos, ventana asociada, etc.
        
        * Un proceso siempre tiene un padre, menos el primero que es el init (PID 1)
            * En espacio de usuario, solo se permite crear procesos o bien hijos tuyos, o bien hijos de init (corriendo en el background, tipo nohup)
            * No se puede matar a init

        * Los descendientes de un proceso son todos los procesos que están por debajo de él en el árbol de procesos
            * Es necesario saber los descendientes porque para los permisos puede interesar que algún archivo esté restringido exclusivamente a los descendientes de un proceso (más que nada para un IPC medianamente seguro)
            * También porque al morir un proceso se matan todos sus descendientes (no permitimos procesos huérfanos, si querés que un proceso no dependa del padre, ejecutalo como nohup)
             
        * Un proceso puede estar en uno de los siguientes estados:
            * NEW: El proceso fue creado pero no se inicializó (lo guardamos de onda, la verdad no se usa realmente)
            * READY: El proceso está listo para ejecutarse, esperando su turno en el scheduler
            * RUNNING: El proceso está actualmente en ejecución
            * WAITING: El proceso está esperando algo (puede ser un semáforo o un evento como teclado, cambio de hora, etc.)
            * TERMINATED: El proceso ha finalizado su ejecución y está listo para ser eliminado
        
        * Cada proceso tiene su STDIN, STDOUT Y STDERR, que son archivos FIFO (pipes)
            * Si el proceso es main, estos archivos se le pasan al ser creado
                -> Y los permisos se setean automáticamente para que el proceso se vuelva el owner de la acción que corresponda (leer o escribir según sea stdin o stdout/stderr)
            * Si el proceso es un thread, hereda los archivos del padre
                -> así que debe ser claro qué worker escribe/lee, o se deben usar mútexes para no generar conflictos
            * Cuando muere un proceso, se cierran sus fifos de escritura ya que no se va a escribir más en ellos
            * Pueden no tener (estar seteados a 0), en ese caso se ignora la entrada/salida (leer la entrada lee un EOF, escribir la salida no hace nada)
            * Quien orquesta estos pipeos, creando y asignando los archivos, es el padre de cada proceso; por ejemplo la shell en el caso más común
                -> Ej. si la shell recibe "A | B", mapearía el input de teclado a stdin de A, stdout de A a stdin de B, y stdout/error de B a la pantalla
                    (Estos mapeos son siempre via archivos FIFO que la shell crea)
*/

void initScheduler();
void schedulerLoop();

// Agrega un proceso main al scheduler, devuelve el PID del proceso agregado o 0 si hubo un error
Pid newProcess(Program program, char *arguments, Priority priority, Pid parent_pid);
Pid newProcessWithIO(Program program, char *arguments, Priority priority, Pid parent_pid, uint64_t stdin, uint64_t stdout, uint64_t stderr);

// Crea un nuevo hilo (thread) para un proceso existente, devuelve el PID del nuevo hilo o 0 si hubo un error
// La diferencia entre thread y proceso es que comparten el mismo espacio de memoria, no tienen una ventana asociada y no pueden tener hijos propios
// Si se crea un thread en otro thread, queda a nombre del proceso padre
Pid newThread(ProgramEntry entrypoint, char *arguments, Priority priority, Pid parent_pid);

// Termina un proceso (o thread) dado, puede ser el actual o no, si es el actual no se va a volver de la función
// Si matás un proceso padre, mata a los hijos también (si hay algo que querés que no dependa del padre, ejecutalo como nohup)
// Retorna si fue exitoso o no
int terminateProcess(Pid pid);  


// Pasa al siguiente proceso de la lista del scheduler
void scheduleNextProcess(); // es un yield básicamente


// Es necesario para saber cosas como en qué buffer de video escribir o darte cuenta de no matar el proceso actual
Pid getCurrentProcessPID();

// Pid 0 es inválido, por eso se devuelve un proceso con pid 0 si no hay proceso con el pid
// En ese caso program queda con basura
Process getProcess(Pid pid); // Devuelve el proceso del pid especificado

// Devuelve el proceso padre del proceso especificado, si devuelve un proceso con pid 0, significa que no hay padre (es el init o no existe el proceso)
Process getParent(Pid pid);

// si no existe el proceso devuelve todo 0, pero debería validarse antes ya que esos pueden ser I/O válidos
IO_Files getIOFiles(Pid pid);

// Retorna 1 si ambos procesos pertenecen al mismo grupo, 0 si no
// Un grupo de procesos es el proceso main y sus threads, ya que para muchas cosas se los considera como un solo proceso
// Por ejemplo, un grupo de procesos comparte stdin/stdout, la ventana gráfica, permisos sobre un archivo, etc.
int isSameProcessGroup(Pid pid1, Pid pid2);

// Devuelve el PID del proceso main del grupo al que pertenece el proceso especificado
Pid getProcessGroupMain(Pid pid);

int detachProcess(Pid pid);

// Devuelve si un proceso dado es descendiente de otro proceso (o sea, si es hijo, hijo de un hijo, thread de un hijo, etc.)
// Si el proceso es el mismo devuelve 1 (por fines prácticos en esta función se toma como que un proceso siempre es descendiente de sí mismo)
int isDescendantOf(Pid child_pid, Pid parent_pid);

Process * getAllProcesses(int * count); // Devuelve una lista de todos los procesos en ejecución (para ps), cuando se encuentre un proceso con pid 0, significa el final de la lista

int changePriority(Pid pid, Priority newPriority); // Cambia la prioridad de un proceso, devuelve -1 si no se pudo cambiar, 0 si se cambió correctamente
Priority getPriority(Pid pid); // Obtiene la prioridad de un proceso, devuelve LOW, NORMAL o HIGH


// --- Semáforos y esperas ---

int setWaiting(Pid pid); // Deja el proceso en espera, si es el actual no se vuelve de esta función ya que se pasa al siguiente proceso
int wakeProcess(Pid pid); // Despierta un proceso que estaba en espera

uint64_t sem_init(int initial_value);   // inicializa un semáforo con el valor especificado, devuelve el id del semáforo creado
int sem_destroy(uint64_t id);           // destruye el semáforo con el id especificado, no se puede destruir si hay procesos esperándolo
void sem_wait(uint64_t id);             // decrementa el semáforo de id especificado, si el valor es menor a cero bloquea el proceso
void sem_post(uint64_t id);             // incrementa el semáforo de id especificado, si el valor es mayor o igual a cero despierta un proceso que estuviera esperando

// Guarda los registros del proceso actual en el backup de registros
// Importante ya que no solo se debe guardar el contexto en cada timertick, sino que debe guardarse en cada interrupción ya que puede no volver por donde vino
// Por ejemplo en una syscall que haga wait del proceso actual se necesita el contexto al que volvería la syscall para poder restaurarlo cuando esté ready
void backupCurrentProcessRegisters(); 

#endif

