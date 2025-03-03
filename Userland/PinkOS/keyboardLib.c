#include <keyboard.h>
#include <syscall.h>
#include <stdint.h>
#include <syscallCodes.h>

KeyboardEvent getKeyboardEvent(){
    KeyboardEvent event;
    syscall(GET_KEY_EVENT_SYSCALL, (uint64_t)&event, 0, 0, 0, 0);
    return event;
}