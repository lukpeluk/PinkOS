#include <stdpink.h>
#include <stdarg.h>
#include <stdin.h>
#include <stdint.h>

static char * invalid_format_message = "Invalid format, use %s for a string or %d for a int\n";

void print(char * string){
    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, string, 0, 0, 0);
}

//----------------------------------------------------------------------------------------------
// HELPERS
//----------------------------------------------------------------------------------------------

// Internal function to convert a number to a string
char * num_to_string(int num) {
    static char buffer[12]; //? Cuanto deberia soportar?
    buffer[11] = '\0';      // Agregar el terminador
    int i = 10;
    int is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    do {
        buffer[i--] = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    if (is_negative) {
        buffer[i--] = '-';
    }

    return &buffer[i + 1];
}

//----------------------------------------------------------------------------------------------
// LIBRARY FUNCTIONS
//----------------------------------------------------------------------------------------------

void strcpy(char * dest, char * src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = 0;
}

void printf(char * format, ...) {
    va_list args;
    va_start(args, format);
    char *str = format;

    while (*str) {
        if (*str == '%') {
            str++;
            switch (*str) {
                case 'd': {
                    int num = va_arg(args, int);
                    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, num_to_string(num), 0, 0, 0);
                    break;
                }
                case 's': {
                    char *string = va_arg(args, char *);
                    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, string, 0, 0, 0);
                    break;
                }
                case '0' ... '9': {
                    int width = 0;
                    while (*str >= '0' && *str <= '9') {
                        width = width * 10 + (*str - '0');
                        str++;
                    }
                    if (*str == 's') {
                        char *string = va_arg(args, char *);
                        int len = 0;
                        while (string[len] != '\0') {
                            len++;
                        }
                        syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, string, 0, 0, 0);
                        for (int i = 0; i < width - len; i++) {
                            syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_CHAR_ENDPOINT, ' ', 0, 0, 0);
                        }
                    } else {
                        syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, invalid_format_message, 0, 0, 0);
                        va_end(args);
                        return;
                    }
                    break;
                }
                default:
                    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, invalid_format_message, 0, 0, 0);                
                    va_end(args);
                    return;
            }
        } else {
            syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_CHAR_ENDPOINT, *str, 0, 0, 0);
        }
        str++;
    }

    va_end(args);
}

void putChar(char c){
    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_CHAR_ENDPOINT, c, 0, 0, 0);
}

void puts(char * string){
    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, string, 0, 0, 0);
    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_CHAR_ENDPOINT, '\n', 0, 0, 0);
}

char getChar(){
    return (char)get_char_from_stdin();
}

//TODO: arreglar el scanf
void scanf(char * format, ...){
    va_list args;
    va_start(args, format);
    char *str = format;

    while (*str) {
        if (*str == '%') {
            str++;
            switch (*str) {
                case 'd': {
                    int *num = va_arg(args, int *);
                    char c;
                    int sign = 1;
                    *num = 0;

                    while ((c = getChar()) != '\n') {
                        if (c == '-') {
                            sign = -1;
                        } else {
                            *num = *num * 10 + c - '0';
                        }
                    }

                    *num *= sign;
                    break;
                }
                case 's': {
                    char *string = va_arg(args, char *);
                    char c;
                    int i = 0;

                    while ((c = getChar()) != '\n') {
                        string[i++] = c;
                    }

                    string[i] = 0;
                    break;
                }
                default:
                    syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, invalid_format_message, 0, 0, 0);
                    va_end(args);
                    return;
            }
        }
        str++;
    }

    va_end(args);
}

void enableBackgroundAudio(){
    syscall(USER_ENVIRONMENT_API_SYSCALL, ENABLE_BACKGROUND_AUDIO_ENDPOINT, 0, 0, 0, 0);
}

void disableBackgroundAudio(){
    syscall(USER_ENVIRONMENT_API_SYSCALL, DISABLE_BACKGROUND_AUDIO_ENDPOINT, 0, 0, 0, 0);
}

void clear(){
    syscall(USER_ENVIRONMENT_API_SYSCALL, CLEAR_SCREEN_ENDPOINT, 0, 0, 0, 0);
}

void sleep(uint64_t millis){
    syscall(SLEEP_SYSCALL, millis, 0, 0, 0, 0);
}