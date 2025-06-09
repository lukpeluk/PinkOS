#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>
#include <permissions.h>
#include <eventHandling/eventHandlers.h>
#include <fileSystem/fileSystem.h>
#include <processManager/scheduler.h>

//TODO: args deberían ser void* para poder pasar cualquier cosa, cada programa sabrá qué recibe (normalmente sería un string pero en el caso de threads podría ser un struct con más información)
typedef void (*ProgramEntry)(char*);


void initProcessState();
InterruptStackFrame getDefaultCRI();

// getters and setters

int isRootMode();
void activateRootMode();
void desactivateRootMode();

uint32_t getPermissions();

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions);


// void loadStackBase(uint64_t stackBase);
// uint64_t getSystemStackBase();

// Pid runProgram(Program * programName, char * arguments);

// void quitProgram(Pid pid);


#endif