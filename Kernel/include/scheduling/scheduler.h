#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <processState.h>
#include <drivers/registersDriver.h>

#define IS_GRAPHIC(currentProcess) (currentProcess->permissions & DRAWING_PERMISSION;)

typedef struct ProcessControlBlock {
    char *name;                     
    uint32_t pid;                   
    uint32_t permissions;           
    uint64_t stackBase;             // Dirección base del stack asignado, no es lo mismo que el rbp guardado ya que eso puede ser modificado por el proceso
    Registers registers;            // Registros del proceso, incluyendo el stack pointer (que en realidad apunta al interrupt stack frame, no al stack del proceso en sí)
    struct ProcessControlBlock *next; // Siguiente proceso (lista circular)
} ProcessControlBlock;

void initScheduler();

void addProcessToScheduler(Program *program, char *arguments);
void removeProcessFromScheduler(uint32_t pid);
void scheduleNextProcess();
void terminateCurrentProcess();
void schedulerLoop();

uint32_t getCurrentProcessPID();

#endif