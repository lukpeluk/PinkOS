#include <kernelState.h>
#include <stdint.h>

// If you are looking for specific state variables like timezone, font size, etc... you should look at the respective drivers

// stores pointers to the handler functions
typedef struct KernelState {
    int kernelMode;       // 1 if the kernel is running in kernel mode, 0 otherwise
    uint32_t permissions; // Permissions for the current process
    char * currentProcess; // Name of the current process (for crash reporting and logging)
} KernelState;

static KernelState kernelState;

// initializes the kernel state
void initKernelState() {
    kernelState.kernelMode = 1;
    kernelState.permissions = ROOT_PERMISSIONS;
    kernelState.currentProcess = "kernel";
}

// getters and setters

int getKernelMode() {
    return kernelState.kernelMode;
}

void setKernelMode(int mode) {
    kernelState.kernelMode = mode;
}

uint32_t getPermissions() {
    return kernelState.permissions;
}

void setPermissions(uint32_t permissions) {
    kernelState.permissions = permissions;
}

char * getCurrentProcess() {
    return kernelState.currentProcess;
}

void setCurrentProcess(char * process) {
    kernelState.currentProcess = process;
}

// recibe un uint32_t con los permisos requeridos y devuelve 1 si el proceso actual tiene esos permisos, 0 en caso contrario
int validatePermissions(uint32_t requiredPermissions) {
    return (kernelState.permissions & requiredPermissions) == requiredPermissions;
}