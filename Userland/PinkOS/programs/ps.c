#include <programs.h>
#include <stdpink.h>

void ps_main(unsigned char *args) {
    //    PID TTY          TIME CMD
    unsigned char *header_pid = "PID";
    unsigned char *header_tty = "TTY";
    unsigned char *header_time = "TIME";
    unsigned char *header_cmd = "CMD";
 //   696969 pts/0    00:00:00 ps
    unsigned char *pid = "696969";
    unsigned char *tty = "pts/0";
    unsigned char *time = "00:00:00";
    unsigned char *cmd = "ps";
    printf("%10s %10s %10s %10s\n", header_pid, header_tty, header_time, header_cmd);
    printf("%10s %10s %10s %10s\n", pid, tty, time, cmd);
    
}