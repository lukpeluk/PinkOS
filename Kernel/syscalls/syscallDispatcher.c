#include <stdint.h>
#include <videoDriver.h>
#include <eventHandlerManager.h>

void syscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall) {
        case 0:
            putPixel(arg1, arg2, arg3);
            break;
        case 1:
            drawChar(arg1, arg2, arg3);
            break;
        case 2:
            clearScreen(arg1);
            break;
        case 3:
            // registers a key press handler with the provided function pointer
            setKeypressHandler((KeyHandler)arg1);
            break;
        default:
            break;
    }
}



