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


extern void magic_recover(InterruptStackFrame * stackBase, uint64_t was_graphic);
extern void push_to_custom_stack_pointer(uint64_t stack_pointer, uint64_t value_to_push);
extern uint64_t get_stack_pointer();
extern void loader();

// stores pointers to the handler functions
typedef struct ProcessState {
    int rootMode;       // 1 if the kernel is running in root mode, 0 otherwise
    uint32_t permissions; // Permissions for the current process
    unsigned char * currentProcess; // Name of the current process (for crash reporting and logging)
    uint64_t systemStackBase; // Base of the system stack
} ProcessState;

static ProcessState processState;

InterruptStackFrame getDefaultCRI() {
    InterruptStackFrame defaultCRI;
    defaultCRI.rip = 0;
    defaultCRI.cs = 0x08;
    defaultCRI.rflags = 0x202;
    defaultCRI.rsp = 0;
    defaultCRI.ss = 0x0;
    return defaultCRI;
}

// initializes the kernel state
void initProcessState() {
    processState.rootMode = ACTIVATE_ROOT_MODE;
    processState.permissions = ROOT_PERMISSIONS;
    processState.currentProcess = (unsigned char *)SYSTEM_PROCESS;
    processState.systemStackBase = 0;
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

unsigned char * getCurrentProcess() {
    return processState.currentProcess;
}

void setCurrentProcess(unsigned char * process) {
    processState.currentProcess = process;
}

void loadStackBase(uint64_t stackBase) {
    processState.systemStackBase = stackBase;
}


void runProgram(Program * program, unsigned char * arguments) {
    desactivateRootMode();
    setPermissions(program->perms);
    setCurrentProcess(program->name);           // Save the name and not the command, because the purpose of this is to be used in crash reports and logs
    
    // ↓↓↓↓ Versión que limpia el stack antes de llamar a la función del programa ↓↓↓↓
    InterruptStackFrame cri = getDefaultCRI();
    cri.rip = program->entry;
    cri.rsp = processState.systemStackBase - 8; // -8 because the return address is pushed in the stack
    push_to_custom_stack_pointer(processState.systemStackBase, (uint64_t)quitProgram);
    magic_recover(&cri, arguments);             

    // ↓↓↓↓ Versión que preserva el stack antes de llamar a la función del programa ↓↓↓↓
    // InterruptStackFrame cri = getDefaultCRI();
    // cri.rip = program->entry;
    // cri.rsp = get_stack_pointer() - 8; 
    // magic_recover(&cri, arguments);
    // quitProgram();
}

void quitProgram() {
    uint64_t was_graphic = processState.permissions & DRAWING_PERMISSION;
    setPermissions(ROOT_PERMISSIONS);
    setCurrentProcess(SYSTEM_PROCESS);
    InterruptStackFrame cri = getDefaultCRI();
    cri.rip = (uint64_t)callRestoreContextHandler;
    cri.rsp = processState.systemStackBase;
    magic_recover(&cri, was_graphic);      // We need to pass the argument to the function that will be called
}

// receives a uint32_t with the required permissions and returns 1 if the current process has those permissions, 0 otherwise
// But if the kernel is in root mode, it returns 1
int validatePermissions(uint32_t requiredPermissions) {
    return processState.rootMode || (processState.permissions & requiredPermissions) == requiredPermissions;
}