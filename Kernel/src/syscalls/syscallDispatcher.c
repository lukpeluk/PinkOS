#include <stdint.h>
#include <eventHandling/eventHandlers.h>
#include <processState.h>
#include <permissions.h>
#include <syscalls/syscallCodes.h>
#include <drivers/pitDriver.h>
#include <drivers/videoDriver.h>
#include <drivers/keyboardDriver.h>
#include <drivers/rtcDriver.h>

#define VALIDATE_PERMISSIONS(permission) if (!validatePermissions(permission)) return

void systemSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
void videoDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
void keyboardDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
void rtcDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);


void syscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    if(syscall < 1000)
        systemSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1100)
        videoDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1200)
        rtcDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1300)
        return;
    else if(syscall < 1400)
        keyboardDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else
        return;
}


// --- SYSTEM SYSCALLS ---
void systemSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
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
void videoDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
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
            int is_cursor_in_boundaries = isCursorInBoundaries(arg1, arg2);
            *(int *)arg3 = is_cursor_in_boundaries;
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
            uint32_t cursor_column =  getCursorColumn();
            *((uint32_t *)arg1) = cursor_column;
            break;
        case GET_CURSOR_LINE_SYSCALL:
            uint32_t cursor_line = getCursorLine();
            *((uint32_t *)arg1) = cursor_line;
            break;
        case GET_SCREEN_WIDTH_SYSCALL:
            uint64_t screen_width = getScreenWidth();
            *((uint64_t *)arg1) = screen_width;
            break;
        case GET_SCREEN_HEIGHT_SYSCALL:
            uint64_t screen_height = getScreenHeight();
            *(uint64_t *)arg1 = screen_height;
            break;
        case GET_CHAR_WIDTH_SYSCALL:
            uint64_t char_width = getCharWidth();
            *(uint64_t *)arg1 = char_width;
            break;
        case GET_CHAR_HEIGHT_SYSCALL:
            uint64_t char_height = getCharHeight();
            *(uint64_t *)arg1 = char_height;
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
        case INC_FONT_SIZE_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_SIZE_PERMISSION);
            incFontSize();
            break;
        case DEC_FONT_SIZE_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_SIZE_PERMISSION);
            decFontSize();
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
void rtcDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case GET_RTC_TIME_SYSCALL:
            get_time((RTC_Time *)arg1);
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
void keyboardDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case GET_KEY_EVENT_SYSCALL:
            getKeyboardEvent(arg1);
            break;
        case CLEAR_KEYBOARD_BUFFER_SYSCALL:
            clearKeyboardBuffer();
            break;
        case IS_KEY_PRESSED_SYSCALL:
            int is_key_pressed = isKeyPressed(arg1, arg2);
            *(int*)arg3 = is_key_pressed; 
            break;

        default:
            break;
    }
}

