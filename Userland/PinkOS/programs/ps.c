#include <programs.h>
#include <stdpink.h>

void ps_main(char *args) {
    //    PID TTY          TIME CMD
    char *header_pid = "PID";
    char *header_tty = "TTY";
    char *header_time = "TIME";
    char *header_cmd = "CMD";
 //   696969 pts/0    00:00:00 ps
    char *pid = "696969";
    char *tty = "pts/0";
    char *time = "00:00:00";
    char *cmd = "ps";
    printf("%10s %10s %10s %10s\n", header_pid, header_tty, header_time, header_cmd);
    printf("%10s %10s %10s %10s\n", pid, tty, time, cmd);
    
}