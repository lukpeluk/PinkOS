#include <libs/stdpink.h>

void block_main(char *args) {
    // Cambia el estado de un proceso entre bloqueado y listo dado su PID.

    if (args == NULL || args[0] == '\0') {
        printf((char *)"Usage: block <pid>\n");
        return;
    }

    Pid pid = (Pid)string_to_uint64(args);
    if (pid <= 0) {
        printf((char *)"Invalid PID: %s\n", args);
        return;
    }

    // Conseguir info del proceso, solamente para mostrar que se está bloqueando
    Process process = getProcess(pid);
    if (process.pid == 0) {
        printf((char *)"No process found with PID %d.\n", pid);
        return;
    }

    if(process.state == PROCESS_STATE_WAITING) {
        printf((char *)"Changing process %d from WAITING to READY.\n", pid);
        wakeProcess(pid);
    }
    else if (process.state == PROCESS_STATE_READY) {
        printf((char *)"Changing process %d from READY to WAITING.\n", pid);
        setWaiting(pid);
    } else {
        printf((char *)"Process %d is not in a state that can be blocked or unblocked.\n", pid);
    }

}