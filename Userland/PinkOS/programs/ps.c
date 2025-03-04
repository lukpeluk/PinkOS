#include <programs.h>
#include <stdpink.h>

void ps_main(unsigned char *args) {
    //    PID TTY          TIME CMD
    unsigned char *header_pid = (unsigned char *) "PID";
    unsigned char *header_tty = (unsigned char *) "TTY";
    unsigned char *header_time = (unsigned char *) "TIME";
    unsigned char *header_cmd = (unsigned char *) "CMD";
 //   696969 pts/0    00:00:00 ps
    seedRandom(getMillisElapsed());
    unsigned char *pid = (unsigned char *) "696969";
    for (int i = 0; i < 6; i++) {
        pid[i] = randInt(0, 9) + '0';
    }

    unsigned char *tty = (unsigned char *) "pts/0";
    unsigned char *time = (unsigned char *) "00:00:00";
    unsigned char *cmd = (unsigned char *) "ps";
    printf((unsigned char *)"%10s %10s %10s %10s\n", header_pid, header_tty, header_time, header_cmd);
    printf((unsigned char *)"%10s %10s %10s %10s\n", pid, tty, time, cmd);
    
}