#include <programs.h>
#include <stdint.h>
#include <syscalls/syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <libs/stdpink.h>

#include <libs/graphicsLib.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void echo_main(char *args){
    print(args);
}