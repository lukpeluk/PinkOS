#include <processManager/processState.h>
#include <stdint.h>
#include <drivers/videoDriver.h>
#include <processManager/scheduler.h>
#include <drivers/serialDriver.h>

int rootMode = 1; // Kernel starts in root mode by default

void initProcessState() {
//     log_to_serial("initProcessState: Iniciando el estado del proceso");
}

// Devuelve un interrupt stack frame "modelo" 
InterruptStackFrame getDefaultCRI() {
    InterruptStackFrame defaultCRI;
    defaultCRI.rip = 0;
    defaultCRI.cs = 0x08;
    defaultCRI.rflags = 0x202;
    defaultCRI.rsp = 0;
    defaultCRI.ss = 0x0;
    return defaultCRI;
}

// getters and setters

int isRootMode() {
    return rootMode;
}
void activateRootMode() {
    rootMode = 1;
}
void desactivateRootMode() {
    rootMode = 0;
}

// Program getCurrentProgram() {
//     Process currentProcess = getCurrentProcess();
//     if(currentProcess.pid) {
//         return currentProcess.program;
//     }
//     return;
// }

uint32_t getPermissions() {
    Process currentProcess = getProcess(getCurrentProcessPID());
    if(currentProcess.pid)
        return currentProcess.program.permissions;
    return 0; // No hay proceso actual, permisos por defecto
}

// Tells if some permision is active, either because the kernel is in root mode or because the process has that permission
// receives a uint32_t with the required permissions and returns 1 if the current process has those permissions, 0 otherwise
int validatePermissions(uint32_t requiredPermissions) {
    uint32_t permissions = getPermissions();
    return rootMode || (permissions & requiredPermissions) == requiredPermissions;
}