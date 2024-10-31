#include <programs.h>
#include <stdint.h>
#include <syscallCodes.h>
extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void echo_main(char *args){
    syscall(DRAW_STRING_SYSCALL,args,0x00df8090,0x00000000,0x00000000,0x00000000);
}