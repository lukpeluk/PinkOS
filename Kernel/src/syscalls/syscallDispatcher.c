#include <stdint.h>
#include <eventHandling/eventHandlers.h>
#include <processState.h>
#include <permissions.h>
#include <syscalls/syscallCodes.h>
#include <drivers/pitDriver.h>
#include <drivers/videoDriver.h>
#include <drivers/keyboardDriver.h>

#define VALIDATE_PERMISSIONS(permission) if (!validatePermissions(permission)) return

uint64_t systemSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t videoDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

uint64_t syscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    if(syscall < 1000)
        return systemSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 2000)
        return videoDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
}


// --- SYSTEM SYSCALLS ---
uint64_t systemSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall) {
        case SET_HANDLER_SYSCALL:
            VALIDATE_PERMISSIONS(SET_HANDLER_PERMISSION);
            registerHandler(arg1, (void *)arg2);
            break;
        case RUN_PROGRAM_SYSCALL:
            VALIDATE_PERMISSIONS(SET_PROCESS_PERMISSION);
            runProgram((Program *)arg1, (char *)arg2);
            break;
        case QUIT_PROGRAM_SYSCALL:
            quitProgram();
            break;
        case USER_ENVIRONMENT_API_SYSCALL:
            callUserEnvironmentApiHandler(arg1, arg2, arg3, arg4, arg5);
            break;
        case SLEEP_SYSCALL:
            sleep(arg1);
            break;
        case SET_SYSTEM_STACK_BASE_SYSCALL:
            loadStackBase(arg1);
            break;

        default:
            break;
    }
}

// --- VIDEO DRIVER SYSCALLS ---
uint64_t videoDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        // BASIC SHAPES
        case DRAW_PIXEL_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            putPixel(arg1, arg2, arg3);
            break;
        case DRAW_RECTANGLE_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawRectangle((Point *)arg1, (Point *)arg2, arg3);
            break;
        case DRAW_RECTANGLE_BORDER_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawRectangleBoder((Point *)arg1, (Point *)arg2, arg3, arg4);
            break; 

        // TEXT
        case DRAW_CHAR_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawChar(arg1, arg2, arg3, 1);
            break;
        case DRAW_STRING_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawString(arg1, arg2, arg3);
            break;
        case DRAW_CHAR_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawCharAt(arg1, arg2, arg3, arg4);
            break;
        case DRAW_STRING_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawStringAt(arg1, arg2, arg3, arg4);
            break;
        case DRAW_NUMBER_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawNumber(arg1, arg2, arg3, 1);
            break;
        case DRAW_NUMBER_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawNumberAt(arg1, arg2, arg3, arg4);
            break;
        case DRAW_HEX_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawHex(arg1, arg2, arg3, 1);
            break;
        case DRAW_HEX_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawHexAt(arg1, arg2, arg3, arg4);
            break;

        // BITMAPS
        case DRAW_BITMAP_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawBitmap((uint32_t *)arg1, arg2, arg3, (Point *)arg4, arg5);
            break;

        // CURSOR
        case IS_CURSOR_IN_BOUNDARIES_SYSCALL:
            return isCursorInBoundaries(arg1, arg2);
            break;

        case SET_CURSOR_COL_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            setCursorColumn(arg1);
            break;
        case SET_CURSOR_LINE_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            setCursorLine(arg1);
            break;
        case GET_CURSOR_COL_SYSCALL:
            return getCursorColumn();
            break;
        case GET_CURSOR_LINE_SYSCALL:
            return getCursorLine();
            break;
        case GET_SCREEN_WIDTH_SYSCALL:
            return getScreenWidth();
            break;
        case GET_SCREEN_HEIGHT_SYSCALL:
            return getScreenHeight();
            break;
        case GET_CHAR_WIDTH_SYSCALL:
            return getCharWidth();
            break;
        case GET_CHAR_HEIGHT_SYSCALL:
            return getCharHeight();
            break;

        // GENERAL

        case CLEAR_SCREEN_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            clearScreen(arg1);
            break;

        case SET_FONT_SIZE_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_SIZE_PERMISSION);
            setFontSize(arg1);
            break;
        case SET_FONT_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_PERMISSION);
            setFont((Font)arg1);
            break;
    
        default:
            break;
    }
}

// --- RTC DRIVER SYSCALLS ---
uint64_t rtcDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case GET_RTC_TIME_SYSCALL:
            return (uint64_t)get_time();
            break;
        case SET_TIMEZONE_SYSCALL:
            VALIDATE_PERMISSIONS(SET_TIMEZONE_PERMISSION);
            set_timezone(arg1);
            break;

        default:
            break;
    }
}

// --- KEYBOARD DRIVER SYSCALLS ---
uint64_t keyboardDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case GET_KEY_EVENT_SYSCALL:
            return getKeyboardEvent();
            break;
        case CLEAR_KEYBOARD_BUFFER_SYSCALL:
            clearKeyboardBuffer();
            break;
        case IS_KEY_PRESSED_SYSCALL:
            return isKeyPressed(arg1, arg2);
            break;

        default:
            break;
    }
}

