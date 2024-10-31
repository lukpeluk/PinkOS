#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>

typedef void (*ProgramEntry)(char*);

typedef struct {
    char* command;
    char* name;
    ProgramEntry entry;
    uint32_t perms;
    char* help;         // This is the help command (a very brief description)
    char* description;  // All the information about the command
} Program;

void initProcessState();

// getters and setters

int isRootMode();
void activateRootMode();
void desactivateRootMode();

uint32_t getPermissions();

char * getCurrentProcess();

void runProgram(Program * programName, char * arguments);

void quitProgram();

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions);

#endif