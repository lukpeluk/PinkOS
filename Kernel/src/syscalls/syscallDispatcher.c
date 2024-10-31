#include <stdint.h>
#include <drivers/videoDriver.h>
#include <eventHandling/eventHandlers.h>
#include <kernelState.h>
#include <permissions.h>
#include <syscalls/syscallCodes.h>

#define VALIDATE_PERMISSIONS(permission) if (!validatePermissions(permission)) return

void syscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall) {
        case DRAW_PIXEL_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            putPixel(arg1, arg2, arg3);
            break;
        case DRAW_CHAR_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawChar(arg1, arg2, arg3);
            break;
        case CLEAR_SCREEN_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            clearScreen(arg1);
            break;
        case DRAW_STRING_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawString(arg1, arg2, arg3);
            break;
        case SET_HANDLER_SYSCALL:
            VALIDATE_PERMISSIONS(SET_HANDLER_PERMISSION);
            registerHandler(arg1, (void *)arg2);
            break;
        default:
            break;
    }
}



