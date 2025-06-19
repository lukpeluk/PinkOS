#include <libs/stdpink.h>

void loop_main(char *args) {
    while (1) {
        Pid pid = getPID();
        printf((char *)"Loop process PID: %d\n", pid);
        sleep(1000); // Espera 1 segundo
    }
}