#include <stdpink.h>
#include <syscallCodes.h>

extern uint64_t syscall(uint64_t syscall, uint64_t arg1);

/// @brief  Converts a string to an integer
/// @param str 
/// @return int
/// @note   It also works with negative numbers
int atoi(const unsigned char * str){
    int res = 0;
    int sign = 1;
    int i = 0;
    if(str[0] == '-'){
        sign = -1;
        i++;
    }
    for(; str[i] != '\0'; i++){
        res = res * 10 + str[i] - '0';
    }
    return sign * res;
}

void set_timezone_main(unsigned char * args){
    if(args[0] == '\0'){
        printf((unsigned char *)"usage: set_timezone <timezone>\n");
        return;
    }
    int timezone = atoi(args);
    if(timezone < -12 || timezone > 12){
        printf((unsigned char *)"Invalid timezone\n");
        return;
    }
    syscall(SET_TIMEZONE_SYSCALL, timezone);
    printf((unsigned char *)"Timezone set to %d\n", timezone);
}

