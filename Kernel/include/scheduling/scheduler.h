#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <processState.h>
#include <drivers/registersDriver.h>

#define IS_GRAPHIC(currentProcess) (currentProcess->permissions & DRAWING_PERMISSION)

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

// devuelve el PID del proceso agregado al scheduler, o 0 si hubo un error
uint32_t addProcessToScheduler(Program *program, char *arguments);

void scheduleNextProcess();

// Si el PID es 0, se mata el proceso actual
// Útil para que un proceso pueda terminarse a sí mismo
void terminateProcess(uint32_t pid);

// A nivel externo no es útil ya que para el usuario todo corre "a la vez", pero es necesario para el kernel para saber cosas como en qué buffer de video escribir
uint32_t getCurrentProcessPID();



#endif