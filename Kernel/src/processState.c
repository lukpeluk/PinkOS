#include <processState.h>
#include <eventHandling/eventHandlers.h>
#include <permissions.h>
#include <stdint.h>
#include <drivers/videoDriver.h>

#define ACTIVATE_ROOT_MODE 1
#define DESACTIVATE_ROOT_MODE 0
#define SYSTEM_PROCESS "system"

// If you are looking for specific state variables like timezone, font size, etc... you should look at the respective drivers

// Struct for the structuring of the "stack int" of the interrupts
typedef struct {
    // uint64_t error;   // Error code
    uint64_t ss;      // Stack Segment
    uint64_t rsp;     // Stack Pointer
    uint64_t rflags;  // Flags Register
    uint64_t cs;      // Code Segment
    uint64_t rip;     // Instruction Pointer
} InterruptStackFrame;


extern void magic_recover(InterruptStackFrame * stackBase, uint8_t was_graphic);
extern void load_stack_int(InterruptStackFrame * stackBase);
extern void loader();

// stores pointers to the handler functions
typedef struct ProcessState {
    int rootMode;       // 1 if the kernel is running in root mode, 0 otherwise
    uint32_t permissions; // Permissions for the current process
    char * currentProcess; // Name of the current process (for crash reporting and logging)
    InterruptStackFrame systemStackBase;
} ProcessState;

static ProcessState processState;

// initializes the kernel state
void initProcessState() {
    processState.rootMode = ACTIVATE_ROOT_MODE;
    processState.permissions = ROOT_PERMISSIONS;
    processState.currentProcess = SYSTEM_PROCESS;
    processState.systemStackBase.rip = 0;
    processState.systemStackBase.cs = 0x08;
    processState.systemStackBase.rflags = 0x202;
    processState.systemStackBase.rsp = 0;
    processState.systemStackBase.ss = 0x0;
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

void loadStackBase(uint64_t stackBase) {
    processState.systemStackBase.rsp = stackBase;
}

void runProgram(Program * program, char * arguments) {
    desactivateRootMode();
    setPermissions(program->perms);
    setCurrentProcess(program->name);           // Save the name and not the command, because the purpose of this is to be used in crash reports and logs
    program->entry(arguments);
    quitProgram();
}

void quitProgram() {
    uint8_t was_graphic = processState.permissions & DRAWING_PERMISSION;
    setPermissions(ROOT_PERMISSIONS);
    setCurrentProcess(SYSTEM_PROCESS);
    processState.systemStackBase.rip = callRestoreContextHandler;
    magic_recover(&processState.systemStackBase, was_graphic);
}

// receives a uint32_t with the required permissions and returns 1 if the current process has those permissions, 0 otherwise
// But if the kernel is in root mode, it returns 1
int validatePermissions(uint32_t requiredPermissions) {
    return processState.rootMode || (processState.permissions & requiredPermissions) == requiredPermissions;
}