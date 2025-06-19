#include <libs/stdpink.h>

void nice_main(char *args) {
    if (args == NULL || args[0] == '\0') {
        printf((char *)"Usage: nice <pid> <new_priority>\n");
        return;
    }

    int pid_int, priority_int;
    
    if(sscanf(args, (char *)"%d %d", &pid_int, &priority_int) != 2) {
        printf((char *)"Usage: nice <pid> <new_priority>\n");
        return;
    }

    Pid pid = (Pid)pid_int;
    Priority new_priority = (Priority)priority_int;

    if (pid <= 0) {
        printf((char *)"Invalid PID: %d\n", pid);
        return;
    }

    // Conseguir info del proceso, solamente para mostrar que se está modificando
    Process process = getProcess(pid);
    if (process.pid == 0) {
        printf((char *)"No process found with PID %d.\n", pid);
        return;
    }
    
    printf((char *)"Changing priority of process with PID %d (%s) from %d to %d...\n", 
           pid, process.program.name, process.priority, new_priority);
    changePriority(pid, new_priority);
    printf((char *)"Priority of process with PID %d changed to %d.\n", pid, new_priority);
}