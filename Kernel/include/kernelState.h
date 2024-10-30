#ifndef _STATE_H
#define _STATE_H

#include <stdint.h>

void initKernelState();

// getters and setters

int getKernelMode();
void setKernelMode(int mode);

uint32_t getPermissions();
void setPermissions(uint32_t permissions);

char * getCurrentProcess();
void setCurrentProcess(char * process);

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions);

#endif