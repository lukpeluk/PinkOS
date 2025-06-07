#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>

typedef void (*ProgramEntry)(char*);


// Struct for the structuring of the "stack int" of the interrupts
typedef struct {
    // uint64_t error;   // Error code
    uint64_t ss;      // Stack Segment
    uint64_t rsp;     // Stack Pointer
    uint64_t rflags;  // Flags Register
    uint64_t cs;      // Code Segment
    uint64_t rip;     // Instruction Pointer
} InterruptStackFrame;


typedef struct {
    char* command;
    char* name;
    ProgramEntry entry;
    uint32_t perms;
    char* help;         // This is the help command (a very brief description)
    char* description;  // All the information about the command
} Program;


void initProcessState();
InterruptStackFrame getDefaultCRI();

// getters and setters

int isRootMode();
void activateRootMode();
void desactivateRootMode();

uint32_t getPermissions();

char * getCurrentProcess();

void loadStackBase(uint64_t stackBase);

void runProgram(Program * programName, char * arguments);

void quitProgram();

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions);

#endif