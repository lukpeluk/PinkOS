#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>

#define ACTIVATE_ROOT_MODE 1
#define DESACTIVATE_ROOT_MODE 0

void initKernelState();

// getters and setters

int isRootMode();
void activateRootMode();
void desactivateRootMode();

uint32_t getPermissions();
void setPermissions(uint32_t permissions);

char * getCurrentProcess();
void setCurrentProcess(char * process);

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions);

#endif