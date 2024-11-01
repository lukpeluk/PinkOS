#include <programs.h>
#include <stdint.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void echo_main(char *args){
    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, args, 0,0, 0);

}