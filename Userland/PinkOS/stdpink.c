#include <stdpink.h>

void print(char * string){
    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, string, 0, 0, 0);
}