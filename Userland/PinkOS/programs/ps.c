#include <programs.h>
#include <libs/stdpink.h>

void ps_main(char *args) {
    //    PID TTY          TIME CMD
    char *header_pid = (char *) "PID";
    char *header_tty = (char *) "TTY";
    char *header_time = (char *) "TIME";
    char *header_cmd = (char *) "CMD";
 //   696969 pts/0    00:00:00 ps
    seedRandom(getMillisElapsed());
    char *pid = (char *) "696969";
    for (int i = 0; i < 6; i++) {
        pid[i] = randInt(0, 9) + '0';
    }

    char *tty = (char *) "pts/0";
    char *time = (char *) "00:00:00";
    char *cmd = (char *) "ps";
    printf((char *)"%10s %10s %10s %10s\n", header_pid, header_tty, header_time, header_cmd);
    printf((char *)"%10s %10s %10s %10s\n", pid, tty, time, cmd);
    
}