#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>

typedef void (*ProgramEntry)(unsigned char*);

typedef struct {
    unsigned char* command;
    unsigned char* name;
    ProgramEntry entry;
    uint32_t perms;
    unsigned char* help;         // This is the help command (a very brief description)
    unsigned char* description;  // All the information about the command
} Program;


void initProcessState();


// getters and setters

int isRootMode();
void activateRootMode();
void desactivateRootMode();

uint32_t getPermissions();

unsigned char * getCurrentProcess();

void loadStackBase(uint64_t stackBase);

void runProgram(Program * programName, unsigned char * arguments);

void quitProgram();

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions);

#endif