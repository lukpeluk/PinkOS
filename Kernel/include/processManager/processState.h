#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>
#include <permissions.h>
#include <eventHandling/eventHandlers.h>
#include <fileSystem/fileSystem.h>

//TODO: args deberían ser void* para poder pasar cualquier cosa, cada programa sabrá qué recibe (normalmente sería un string pero en el caso de threads podría ser un struct con más información)
typedef void (*ProgramEntry)(char*);

#define SMALL_TEXT_SIZE 64
#define MEDIUM_TEXT_SIZE 56

// CRI (Context recovery info) -> nos re inventamos el nombre el cuatri pasado lol, cosas de querer implementar pseudo procesos antes de cursar SO
typedef struct {
    uint64_t ss;      // Stack Segment
    uint64_t rsp;     // Stack Pointer
    uint64_t rflags;  // Flags Register
    uint64_t cs;      // Code Segment
    uint64_t rip;     // Instruction Pointer
} InterruptStackFrame;

typedef struct {
    char command[SMALL_TEXT_SIZE];
    char name[SMALL_TEXT_SIZE];
    ProgramEntry entry;
    uint32_t permissions;
    char help[MEDIUM_TEXT_SIZE];  // A very brief description
    char* description;  // All the information about the command (deprecated by the man page file)
    File man_page; // A publically readable but unmodifiable file with the program's manual
} Program;


void initProcessState();
InterruptStackFrame getDefaultCRI();

// getters and setters

int isRootMode();
void activateRootMode();
void desactivateRootMode();

uint32_t getPermissions();

void loadStackBase(uint64_t stackBase);
uint64_t getSystemStackBase();

Pid runProgram(Program * programName, char * arguments);

void quitProgram(Pid pid);

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions);


#endif