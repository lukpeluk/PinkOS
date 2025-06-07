#include <keyboard.h>
#include <stdint.h>
#include <syscallCodes.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

KeyboardEvent getKeyboardEvent(){
    KeyboardEvent event;
    syscall(GET_KEY_EVENT_SYSCALL, (uint64_t)&event, 0, 0, 0, 0);
    return event;
}