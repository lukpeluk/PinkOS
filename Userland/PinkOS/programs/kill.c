#include <libs/stdpink.h>

void kill_main(char *args) {
    if (args == NULL || args[0] == '\0') {
        printf((char *)"Usage: kill <pid>\n");
        return;
    }

    Pid pid = (Pid)atoi(args);
    if (pid <= 0) {
        printf((char *)"Invalid PID: %s\n", args);
        return;
    }

    // Conseguir info del proceso, solamente para mostrar que se está matando
    // Si no se encuentra el proceso, se mostrará un mensaje de error
    Process process = getProcess(pid);
    if (process.pid == 0) {
        printf((char *)"No process found with PID %d.\n", pid);
        return;
    }
    printf((char *)"Killing process with PID %d (%s)...\n", pid, process.program.name);

    killProcess(pid);
    printf((char *)"Process with PID %d killed.\n", pid);
}