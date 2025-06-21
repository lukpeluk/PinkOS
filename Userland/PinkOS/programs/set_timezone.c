#include <libs/stdpink.h>
#include <syscalls/syscallCodes.h>

extern uint64_t syscall(uint64_t syscall, uint64_t arg1);


void set_timezone_main(char * args){
    if(args[0] == '\0'){
        printf((char *)"usage: set_timezone <timezone>\n");
        return;
    }
    int timezone = atoi(args);
    if(timezone < -12 || timezone > 12){
        printf((char *)"Invalid timezone\n");
        return;
    }
    syscall(SET_TIMEZONE_SYSCALL, timezone);
    printf((char *)"Timezone set to %d\n", timezone);
}

