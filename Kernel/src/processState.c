#include <processState.h>
#include <eventHandling/eventHandlers.h>
#include <permissions.h>
#include <stdint.h>

#define ACTIVATE_ROOT_MODE 1
#define DESACTIVATE_ROOT_MODE 0
#define SYSTEM_PROCESS "system"

// If you are looking for specific state variables like timezone, font size, etc... you should look at the respective drivers



// stores pointers to the handler functions
typedef struct ProcessState {
    int rootMode;       // 1 if the kernel is running in root mode, 0 otherwise
    uint32_t permissions; // Permissions for the current process
    char * currentProcess; // Name of the current process (for crash reporting and logging)
} ProcessState;

static ProcessState processState;

// initializes the kernel state
void initProcessState() {
    processState.rootMode = ACTIVATE_ROOT_MODE;
    processState.permissions = ROOT_PERMISSIONS;
    processState.currentProcess = SYSTEM_PROCESS;
}

// getters and setters

int isRootMode() {
    return processState.rootMode;
}

void activateRootMode() {
    processState.rootMode = ACTIVATE_ROOT_MODE;
}

void desactivateRootMode() {
    processState.rootMode = DESACTIVATE_ROOT_MODE;
}   

uint32_t getPermissions() {
    return processState.permissions;
}

void setPermissions(uint32_t permissions) {
    processState.permissions = permissions;
}

char * getCurrentProcess() {
    return processState.currentProcess;
}

void setCurrentProcess(char * process) {
    processState.currentProcess = process;
}

void runProgram(Program * program, char * arguments) {
    desactivateRootMode();
    setPermissions(program->perms);
    setCurrentProcess(program->name);           // Save the name and not the command, because the purpose of this is to be used in crash reports and logs
    program->entry(arguments);
    quitProgram();
}

void quitProgram() {
    // TODO: Clear stack
    uint8_t was_graphic = processState.permissions & DRAWING_PERMISSION;
    setPermissions(ROOT_PERMISSIONS);
    setCurrentProcess(SYSTEM_PROCESS);
    callRestoreContextHandler(was_graphic);
}

// receives a uint32_t with the required permissions and returns 1 if the current process has those permissions, 0 otherwise
// But if the kernel is in root mode, it returns 1
int validatePermissions(uint32_t requiredPermissions) {
    return processState.rootMode || (processState.permissions & requiredPermissions) == requiredPermissions;
}