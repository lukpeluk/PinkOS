#include <stdint.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

// program that runs forever
void forrest_gump_main(char *args) {
    while (1)
    {
        syscall(SLEEP_SYSCALL, 500, 0, 0, 0, 0);
        syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, "run forest run\n",0x00df8090,0x00000000, 0);
    }

}