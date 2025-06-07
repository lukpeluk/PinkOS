#include <stdint.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdpink.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

// program that runs forever
void forrest_gump_main(char *args) {
    while (1)                               // ! Oh oh... infinite loop happening here
    {                                       // ! If only there was a way to stop it in my single-process OS...
        char message []= "run forest run\n";
        if(*args) {
            print(args);
            print("\n");
        } else {
            print(message);
        }
        syscall(DRAW_CHAR_SYSCALL, 'F', 0x00df8090, 0x00000000, 1, 0);  // ! Wow wow wow, this program has permission to draw directly to screen???
        syscall(SLEEP_SYSCALL, 500, 0, 0, 0, 0);
    }
}