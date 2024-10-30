#include <kernelState.h>
#include <permissions.h>
#include <stdint.h>

// If you are looking for specific state variables like timezone, font size, etc... you should look at the respective drivers

// stores pointers to the handler functions
typedef struct KernelState {
    int rootMode;       // 1 if the kernel is running in root mode, 0 otherwise
    uint32_t permissions; // Permissions for the current process
    char * currentProcess; // Name of the current process (for crash reporting and logging)
} KernelState;

static KernelState kernelState;

// initializes the kernel state
void initKernelState() {
    kernelState.rootMode = 1;
    kernelState.permissions = ROOT_PERMISSIONS;
    kernelState.currentProcess = "kernel";
}

// getters and setters

int isRootMode() {
    return kernelState.rootMode;
}

void activateRootMode() {
    kernelState.rootMode = ACTIVATE_ROOT_MODE;
}

void desactivateRootMode() {
    kernelState.rootMode = DESACTIVATE_ROOT_MODE;
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

// receives a uint32_t with the required permissions and returns 1 if the current process has those permissions, 0 otherwise
// But if the kernel is in root mode, it returns 1
int validatePermissions(uint32_t requiredPermissions) {
    return kernelState.rootMode || (kernelState.permissions & requiredPermissions) == requiredPermissions;
}