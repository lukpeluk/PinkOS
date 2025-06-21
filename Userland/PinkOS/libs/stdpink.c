#include <libs/stdpink.h>
#include <syscalls/syscall.h>
#include <libs/events.h>

#include <stdarg.h>
#include <stdin.h>
#include <stdint.h>

// Define missing syscall if not already defined
#ifndef USER_ENVIRONMENT_API_SYSCALL
#define USER_ENVIRONMENT_API_SYSCALL 3
#endif


static uint32_t randmon_seed = 0;

static char * invalid_format_message = (char *) "Invalid format, use %s for a string or %d for a int\n";

// New functions

// ====== Basic Functions ======

uint64_t memcmp(const void *s1, const void *s2, uint64_t n) {
    const char *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void * memcpy(void * dest, const void * src, uint64_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void * memset(void * s, int c, uint64_t n) {
    char *p = s;
    while (n--) {
        *p++ = c;
    }
    return s;
}

// ====== String Functions ======

int strcmp(const char * s1, const char * s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int strncmp(const char * s1, const char * s2, uint64_t n) {
    while (n-- && *s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return n == (uint64_t)-1 ? 0 : *s1 - *s2;
}

uint64_t strlen(const char * s) {
    uint64_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

// ====== System Functions ======

// *** Process Management ***

Pid runProgram(char * program, char * args, Priority priority, IO_Files * io_files, int nohup) {
    uint64_t new_pid = (uint64_t) nohup;
    syscall(RUN_PROGRAM_SYSCALL, (uint64_t)program, (uint64_t)args, (uint64_t)priority, (uint64_t)io_files, &new_pid);
    return (Pid)new_pid;
}

Pid newThread(ProgramEntry entrypoint, char * args, Priority priority) {
    return (Pid)syscall(NEW_THREAD_SYSCALL, (uint64_t)entrypoint, (uint64_t)args, (uint64_t)priority, 0, 0);
}

void quit() {
    syscall(QUIT_SYSCALL, 0, 0, 0, 0, 0);
}

void killProcess(Pid pid) {
    syscall(KILL_PROCESS_SYSCALL, (uint64_t)pid, 0, 0, 0, 0);
}

void changePriority(Pid pid, Priority priority) {
    syscall(CHANGE_PRIORITY_SYSCALL, (uint64_t)pid, (uint64_t)priority, 0, 0, 0);
}

void yield() {
    syscall(YIELD_SYSCALL, 0, 0, 0, 0, 0);
}

int setWaiting(Pid pid) {
    return syscall(SET_WAITING_SYSCALL, (uint64_t)pid, 0, 0, 0, 0);
}

void wakeProcess(Pid pid) {
    syscall(WAKE_PROCESS_SYSCALL, (uint64_t)pid, 0, 0, 0, 0);
}

Pid getPID() {
    return (Pid)syscall(GET_PID_SYSCALL, 0, 0, 0, 0, 0);
}

Process getProcess(Pid pid) {
    Process process;
    syscall(GET_PROCESS_INFO_SYSCALL, (uint64_t)pid, (uint64_t)&process, 0, 0, 0);
    return process;
}

Process * getAllProcesses(int * count) {
    return (Process *)syscall(GET_PROCESS_LIST_SYSCALL, (uint64_t)count, 0, 0, 0, 0);
}

Pid getProcessGroupMain() {
    return (Pid)syscall(GET_GROUP_MAIN_PID_SYSCALL, 0, 0, 0, 0, 0);
}

Process * getProcessByCommand(char * command) {
    // This would need a specific syscall, for now return NULL
    return 0;
}

Program * getAllPrograms(int * count) {
    return (Program *)syscall(GET_PROGRAM_LIST_SYSCALL, (uint64_t)count, 0, 0, 0, 0);
}

Program * searchProgramByPrefix(char * command) {
    return (Program *)syscall(SEARCH_PROGRAM_SYSCALL, (uint64_t)command, 0, 0, 0, 0);
}

int installProgram(Program * program) {
    return (int)syscall(INSTALL_PROGRAM_SYSCALL, (uint64_t)program, 0, 0, 0, 0);
}

int uninstallProgramByCommand(const char * command) {
    return (int)syscall(UNINSTALL_PROGRAM_SYSCALL, (uint64_t)command, 0, 0, 0, 0);
}

// *** Process I/O ***

int readStdin(void * buffer, uint32_t size) {
    uint64_t result = size;
    syscall(READ_STDIN, (uint64_t)buffer, &result, 0, 0, 0);
    return result;
}

int writeStdout(const void * buffer, uint32_t size) {
    return (int)syscall(WRITE_STDOUT, (uint64_t)buffer, (uint64_t)size, 0, 0, 0);
}

int writeStderr(const void * buffer, uint32_t size) {
    return (int)syscall(WRITE_STDERR, (uint64_t)buffer, (uint64_t)size, 0, 0, 0);
}

// *** Semaphore Management ***

uint64_t sem_init(int initial_value) {
    uint64_t result;
    syscall(SEM_INIT_SYSCALL, (uint64_t)initial_value, (uint64_t)&result, 0, 0, 0);
    return result;
}

int sem_destroy(uint64_t id) {
    int result;
    syscall(SEM_DESTROY_SYSCALL, id, (uint64_t)&result, 0, 0, 0);
    return result;
}

void sem_wait(uint64_t id) {
    syscall(SEM_WAIT_SYSCALL, id, 0, 0, 0, 0);
}

void sem_post(uint64_t id) {
    syscall(SEM_POST_SYSCALL, id, 0, 0, 0, 0);
}

// *** Event Handling ***

void subscribeToEvent(int event_id, void (*callback)(void *), void * condition_data) {
    syscall(SUSCRIBE_TO_EVENT_SYSCALL, (uint64_t)event_id, (uint64_t)callback, (uint64_t)condition_data, 0, 0);
}

void unsubscribeFromEvent(int event_id) {
    syscall(UNSUBSCRIBE_TO_EVENT_SYSCALL, (uint64_t)event_id, 0, 0, 0, 0);
}

void waitForEvent(int event_id, void * data, void * condition_data) {
    syscall(WAIT_EVENT_SYSCALL, (uint64_t)event_id, (uint64_t)data, (uint64_t)condition_data, 0, 0);
}

// *** File System Management ***

// El archivo se crea con el proceso actual como dueño
// Lo puede instacambiar con setFilePermissions
uint64_t mkFile(char * path, FileType type, uint32_t size) {
    Pid pid = getPID();

    FilePermissions permissions = {
        .writing_owner = pid, 
        .writing_conditions = '.',
        .reading_owner = pid, 
        .reading_conditions = '.'
    };

    return syscall(MK_FILE_SYSCALL, (uint64_t)path, (uint64_t)type, (uint64_t)size, (uint64_t)&permissions, 0);
}

int rmFile(uint64_t id) {
    return (int)syscall(RM_FILE_SYSCALL, id, 0, 0, 0, 0);
}

uint64_t openFile(char * path, FileAction action, FileType type) {
    return syscall(OPEN_FILE_SYSCALL, (uint64_t)path, (uint64_t)action, (uint64_t)type, 0, 0);
}

int closeFile(uint64_t id, Pid pid) {
    return (int)syscall(CLOSE_FILE_SYSCALL, id, (uint64_t)pid, 0, 0, 0);
}

int closeFifoForWriting(uint64_t id) {
    return (int)syscall(CLOSE_FIFO_FOR_WRITING_SYSCALL, id, 0, 0, 0, 0);
}

int readRaw(uint64_t id, void * buffer, uint32_t size, uint32_t offset) {
    return syscall(READ_RAW_FILE_SYSCALL, id, (uint64_t)buffer, (uint64_t)size, (uint64_t)offset, 0);
}

int writeRaw(uint64_t id, void * buffer, uint32_t size, uint32_t offset) {
    return syscall(WRITE_RAW_FILE_SYSCALL, id, (uint64_t)buffer, (uint64_t)size, (uint64_t)offset, 0);
}

int readFifo(uint64_t id, void * buffer, uint32_t size) {
    uint64_t result = size;
    syscall(READ_FIFO_FILE_SYSCALL, id, (uint64_t)buffer, &result, 0, 0);   // Mario bros cloud vibes
    return result;
}

int writeFifo(uint64_t id, void * buffer, uint32_t size) {
    return syscall(WRITE_FIFO_FILE_SYSCALL, id, (uint64_t)buffer, (uint64_t)size, 0, 0);
}

File getFileById(uint64_t id) {
    File file;
    syscall(GET_FILE_SYSCALL, id, (uint64_t)&file, 0, 0, 0);
    return file;
}

File * getFileList(int * count) {
    File * files;
    syscall(GET_FILE_LIST_SYSCALL, (uint64_t)&files, (uint64_t)count, 0, 0, 0);
    return files;
}

int setFilePermissions(uint64_t id, FilePermissions permissions) {
    return syscall(SET_FILE_PERMISSIONS_SYSCALL, id, (uint64_t)&permissions, 0, 0, 0);
}

FilePermissions getFilePermissions(uint64_t id) {
    FilePermissions permissions;
    syscall(GET_FILE_PERMISSIONS_SYSCALL, id, (uint64_t)&permissions, 0, 0, 0);
    return permissions;
}

int validateFileAccessPermissions(char * path, Pid pid, FileAction action) {
    int result;
    syscall(VALIDATE_FILE_ACCESS_PERMISSIONS_SYSCALL, (uint64_t)path, (uint64_t)pid, (uint64_t)action, (uint64_t)&result, 0);
    return result;
}

// *** Window Management ***

void switchToWindow(Pid pid) {
    syscall(SWITCH_WINDOW_SYSCALL, (uint64_t)pid, 0, 0, 0, 0);
}

Pid * getWindows(int * count) {
    Pid * windows;
    syscall(GET_WINDOW_LIST_SYSCALL, (uint64_t)&windows, (uint64_t)count, 0, 0, 0);
    return windows;
}

Pid getFocusedWindow() {
    Pid result;
    syscall(GET_FOCUSED_WINDOW_SYSCALL, (uint64_t)&result, 0, 0, 0, 0);
    return result;
}

int isFocusedWindow(Pid pid) {
    int result;
    syscall(IS_FOCUSED_WINDOW_SYSCALL, (uint64_t)pid, (uint64_t)&result, 0, 0, 0);
    return result;
}


void * malloc(uint64_t size) {
    void * ptr;
    syscall(ALLOCATE_MEMORY_SYSCALL, size, (uint64_t)&ptr, 0, 0, 0);
    return ptr;
}

void free(void * ptr) {
    syscall(FREE_MEMORY_SYSCALL, (uint64_t)ptr, 0, 0, 0, 0);
}


// End new functions


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

// New number conversion functions

// Convert int to string (wrapper for existing num_to_string for consistency)
char * int_to_string(int num) {
    return num_to_string(num);
}

// Convert uint64_t to string
char * uint64_to_string(uint64_t num) {
    static char buffer[21]; // Enough for max uint64_t (20 digits + null terminator)
    buffer[20] = '\0';
    int i = 19;

    if (num == 0) {
        buffer[i--] = '0';
        return &buffer[i + 1];
    }

    while (num > 0) {
        buffer[i--] = (num % 10) + '0';
        num /= 10;
    }

    return &buffer[i + 1];
}

// Convert string to int (similar to atoi)
int string_to_int(const char * str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Skip leading whitespace
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i++;
    }

    // Handle sign
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Convert digits
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return sign * result;
}

// Convert string to uint64_t
uint64_t string_to_uint64(const char * str) {
    uint64_t result = 0;
    int i = 0;

    // Skip leading whitespace
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i++;
    }

    // Convert digits
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return result;
}

char * concatenate_strings(const char * str1, const char * str2) {
    uint64_t len1 = strlen(str1);
    uint64_t len2 = strlen(str2);
    char * result = (char *)malloc(len1 + len2 + 1); // +1 for null terminator

    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    memcpy(result, str1, len1);
    memcpy(result + len1, str2, len2);
    result[len1 + len2] = '\0'; // Null-terminate the string

    return result;
}


//----------------------------------------------------------------------------------------------
// LIBRARY FUNCTIONS
//----------------------------------------------------------------------------------------------

// void strcpy(char * dest, char * src) {
//     while (*src) {
//         *dest = *src;
//         dest++;
//         src++;
//     }
//     *dest = 0;
// }


void print(char * string){
    int a = writeStdout(string, strlen(string));
    log_decimal("STDOUT: ", a);
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
                    char * num_str = num_to_string(num);
                    writeStdout(num_str, strlen(num_str));
                    break;
                }
                case 'u': {
                    uint64_t num = va_arg(args, uint64_t);
                    char * num_str = uint64_to_string(num);
                    writeStdout(num_str, strlen(num_str));
                    break;
                }
                case 's': {
                    char *string = va_arg(args, char *);
                    writeStdout(string, strlen(string));
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    writeStdout(&c, 1);
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
                        writeStdout(string, len);
                        for (int i = 0; i < width - len; i++) {
                            char space = ' ';
                            writeStdout(&space, 1);
                        }
                    } else if (*str == 'd') {
                        int num = va_arg(args, int);
                        char * num_str = num_to_string(num);
                        int len = strlen(num_str);
                        writeStdout(num_str, len);
                        for (int i = 0; i < width - len; i++) {
                            char space = ' ';
                            writeStdout(&space, 1);
                        }
                    } else if (*str == 'c') {
                        char c = (char)va_arg(args, int);
                        writeStdout(&c, 1);
                        for (int i = 0; i < width - 1; i++) {
                            char space = ' ';
                            writeStdout(&space, 1);
                        }
                    } 
                    else {
                        writeStderr(invalid_format_message, strlen(invalid_format_message));
                        va_end(args);
                        return;
                    }
                    break;
                }
                default:
                    writeStderr(invalid_format_message, strlen(invalid_format_message));
                    va_end(args);
                    return;
            }
        } else {
            writeStdout(str, 1);
        }
        str++;
    }

    va_end(args);
}

int sprintf(char * buffer, char * format, ...) {
    va_list args;
    va_start(args, format);
    char *str = format;
    char *buf_ptr = buffer;
    int chars_written = 0;

    while (*str) {
        if (*str == '%') {
            str++;
            switch (*str) {
                case 'd': {
                    int num = va_arg(args, int);
                    char * num_str = num_to_string(num);
                    int len = strlen(num_str);
                    memcpy(buf_ptr, num_str, len);
                    buf_ptr += len;
                    chars_written += len;
                    break;
                }
                case 's': {
                    char *string = va_arg(args, char *);
                    int len = strlen(string);
                    memcpy(buf_ptr, string, len);
                    buf_ptr += len;
                    chars_written += len;
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *buf_ptr++ = c;
                    chars_written++;
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
                        int len = strlen(string);
                        memcpy(buf_ptr, string, len);
                        buf_ptr += len;
                        chars_written += len;
                        for (int i = 0; i < width - len; i++) {
                            *buf_ptr++ = ' ';
                            chars_written++;
                        }
                    } else if (*str == 'd') {
                        int num = va_arg(args, int);
                        char * num_str = num_to_string(num);
                        int len = strlen(num_str);
                        memcpy(buf_ptr, num_str, len);
                        buf_ptr += len;
                        chars_written += len;
                        for (int i = 0; i < width - len; i++) {
                            *buf_ptr++ = ' ';
                            chars_written++;
                        }
                    } else if (*str == 'c') {
                        char c = (char)va_arg(args, int);
                        *buf_ptr++ = c;
                        chars_written++;
                        for (int i = 0; i < width - 1; i++) {
                            *buf_ptr++ = ' ';
                            chars_written++;
                        }
                    } 
                    else {
                        // Invalid format - copy error message to buffer
                        char *error_msg = "Invalid format";
                        int error_len = strlen(error_msg);
                        memcpy(buf_ptr, error_msg, error_len);
                        buf_ptr += error_len;
                        chars_written += error_len;
                        va_end(args);
                        *buf_ptr = '\0';
                        return chars_written;
                    }
                    break;
                }
                default:
                    // Invalid format - copy error message to buffer
                    char *error_msg = "Invalid format";
                    int error_len = strlen(error_msg);
                    memcpy(buf_ptr, error_msg, error_len);
                    buf_ptr += error_len;
                    chars_written += error_len;
                    va_end(args);
                    *buf_ptr = '\0';
                    return chars_written;
            }
        } else {
            *buf_ptr++ = *str;
            chars_written++;
        }
        str++;
    }

    va_end(args);
    *buf_ptr = '\0';  // Null terminate the buffer
    return chars_written;
}

void putChar(char c){
    writeStdout(&c, 1);
}

void puts(char * string){
    writeStdout(string, strlen(string));
    char newline = '\n';
    writeStdout(&newline, 1);
}

char getChar(){
    char c;
    readStdin(&c, 1);
    return c;
}

// void clearStdinBuffer(){
//     clear_stdin();
// }


// Helper function to skip whitespace characters
static void skip_whitespace() {
    char c;
    while ((c = getChar()) == ' ' || c == '\t' || c == '\n' || c == '\r') {
        // Skip whitespace
    }
    // Put back the non-whitespace character
    // Note: We need a way to unget a character, for now we'll work around this
}

void scanf(char * format, ...){
    va_list args;
    va_start(args, format);
    char *str = format;
    char input_buffer[256]; // Buffer to read input line
    int buffer_index = 0;
    int input_index = 0;
    int input_length = 0;
    
    // Read entire input line into buffer
    char c;
    while ((c = getChar()) != '\n' && buffer_index < 255) {
        input_buffer[buffer_index++] = c;
    }
    input_buffer[buffer_index] = '\0';
    input_length = buffer_index;

    while (*str && input_index < input_length) {
        if (*str == '%') {
            str++;
            switch (*str) {
                case 'd': {
                    int *num = va_arg(args, int *);
                    int sign = 1;
                    *num = 0;
                    
                    // Skip whitespace
                    while (input_index < input_length && 
                           (input_buffer[input_index] == ' ' || input_buffer[input_index] == '\t')) {
                        input_index++;
                    }
                    
                    // Handle sign
                    if (input_index < input_length && input_buffer[input_index] == '-') {
                        sign = -1;
                        input_index++;
                    } else if (input_index < input_length && input_buffer[input_index] == '+') {
                        input_index++;
                    }
                    
                    // Parse digits
                    while (input_index < input_length && 
                           input_buffer[input_index] >= '0' && input_buffer[input_index] <= '9') {
                        *num = *num * 10 + (input_buffer[input_index] - '0');
                        input_index++;
                    }
                    
                    *num *= sign;
                    break;
                }
                case 's': {
                    char *string = va_arg(args, char *);
                    int str_index = 0;
                    
                    // Skip whitespace
                    while (input_index < input_length && 
                           (input_buffer[input_index] == ' ' || input_buffer[input_index] == '\t')) {
                        input_index++;
                    }
                    
                    // Read word until whitespace or end of input
                    while (input_index < input_length && 
                           input_buffer[input_index] != ' ' && input_buffer[input_index] != '\t') {
                        string[str_index++] = input_buffer[input_index++];
                    }
                    
                    string[str_index] = '\0';
                    break;
                }
                case 'c': {
                    char *character = va_arg(args, char *);
                    
                    // Skip whitespace
                    while (input_index < input_length && 
                           (input_buffer[input_index] == ' ' || input_buffer[input_index] == '\t')) {
                        input_index++;
                    }
                    
                    // Read single character
                    if (input_index < input_length) {
                        *character = input_buffer[input_index++];
                    }
                    break;
                }
                default:
                    writeStderr(invalid_format_message, strlen(invalid_format_message));
                    va_end(args);
                    return;
            }
        } else if (*str == ' ' || *str == '\t') {
            // Skip whitespace in format string
            while (*str == ' ' || *str == '\t') {
                str++;
            }
            // Skip whitespace in input
            while (input_index < input_length && 
                   (input_buffer[input_index] == ' ' || input_buffer[input_index] == '\t')) {
                input_index++;
            }
            continue;
        } else {
            // Match literal character
            if (input_index < input_length && input_buffer[input_index] == *str) {
                input_index++;
            }
        }
        str++;
    }

    va_end(args);
}

// Helper function to allocate memory for string (simplified malloc)
static char * allocate_string(int length) {
    // For now, we'll use a static buffer approach since we don't have malloc
    // In a real implementation, you'd use malloc
    static char string_buffers[10][256]; // Support up to 10 strings of 256 chars each
    static int current_buffer = 0;
    
    if (length >= 256) return 0; // String too long
    
    char * result = string_buffers[current_buffer];
    current_buffer = (current_buffer + 1) % 10; // Circular buffer
    return result;
}

// sscanf implementation
int sscanf(const char * input, const char * format, ...) {
    va_list args;
    va_start(args, format);
    
    int input_index = 0;
    int format_index = 0;
    int parsed_items = 0;
    int input_length = strlen(input);
    int format_length = strlen(format);
    
    while (format_index < format_length && input_index < input_length) {
        if (format[format_index] == '%') {
            format_index++; // Skip '%'
            
            switch (format[format_index]) {
                case 'd': { // Parse integer
                    int *num_ptr = va_arg(args, int *);
                    int sign = 1;
                    int num = 0;
                    
                    // Skip whitespace
                    while (input_index < input_length && 
                           (input[input_index] == ' ' || input[input_index] == '\t')) {
                        input_index++;
                    }
                    
                    // Handle sign
                    if (input_index < input_length && input[input_index] == '-') {
                        sign = -1;
                        input_index++;
                    } else if (input_index < input_length && input[input_index] == '+') {
                        input_index++;
                    }
                    
                    // Parse digits
                    int digit_found = 0;
                    while (input_index < input_length && 
                           input[input_index] >= '0' && input[input_index] <= '9') {
                        num = num * 10 + (input[input_index] - '0');
                        input_index++;
                        digit_found = 1;
                    }
                    
                    if (digit_found) {
                        *num_ptr = num * sign;
                        parsed_items++;
                    }
                    break;
                }
                
                case 'u': { // Parse uint64_t
                    uint64_t *num_ptr = va_arg(args, uint64_t *);
                    uint64_t num = 0;
                    
                    // Skip whitespace
                    while (input_index < input_length && 
                           (input[input_index] == ' ' || input[input_index] == '\t')) {
                        input_index++;
                    }
                    
                    // Parse digits
                    int digit_found = 0;
                    while (input_index < input_length && 
                           input[input_index] >= '0' && input[input_index] <= '9') {
                        num = num * 10 + (input[input_index] - '0');
                        input_index++;
                        digit_found = 1;
                    }
                    
                    if (digit_found) {
                        *num_ptr = num;
                        parsed_items++;
                    }
                    break;
                }
                
                case 's': { // Parse string/word
                    char **string_ptr = va_arg(args, char **);
                    
                    // Skip whitespace
                    while (input_index < input_length && 
                           (input[input_index] == ' ' || input[input_index] == '\t')) {
                        input_index++;
                    }
                    
                    // Check what character follows %s in format
                    char next_char = 0;
                    if (format_index + 1 < format_length) {
                        next_char = format[format_index + 1];
                    }
                    
                    // Count characters until whitespace, next format character, or end
                    int start_index = input_index;
                    int word_length = 0;
                    while (input_index < input_length) {
                        char current_char = input[input_index];
                        
                        // Stop at whitespace
                        if (current_char == ' ' || current_char == '\t') {
                            break;
                        }
                        
                        // Stop if we find the next literal character from format
                        if (next_char != 0 && current_char == next_char) {
                            break;
                        }
                        
                        word_length++;
                        input_index++;
                    }
                    
                    if (word_length > 0) {
                        // Allocate memory for the string
                        char * allocated_string = allocate_string(word_length + 1);
                        if (allocated_string) {
                            // Copy the word
                            memcpy(allocated_string, &input[start_index], word_length);
                            allocated_string[word_length] = '\0';
                            *string_ptr = allocated_string;
                            parsed_items++;
                        }
                    }
                    break;
                }
                
                case 'c': { // Parse character
                    char *char_ptr = va_arg(args, char *);
                    
                    // Skip whitespace
                    while (input_index < input_length && 
                           (input[input_index] == ' ' || input[input_index] == '\t')) {
                        input_index++;
                    }
                    
                    // Read single character
                    if (input_index < input_length) {
                        *char_ptr = input[input_index];
                        input_index++;
                        parsed_items++;
                    }
                    break;
                }
                
                default:
                    // Unknown format specifier
                    va_end(args);
                    return parsed_items;
            }
            
            format_index++; // Move past the format specifier
            
        } else if (format[format_index] == ' ' || format[format_index] == '\t') {
            // Skip whitespace in format string
            while (format_index < format_length && 
                   (format[format_index] == ' ' || format[format_index] == '\t')) {
                format_index++;
            }
            
            // Skip whitespace in input
            while (input_index < input_length && 
                   (input[input_index] == ' ' || input[input_index] == '\t')) {
                input_index++;
            }
            
        } else {
            // Match literal character
            if (input_index < input_length && input[input_index] == format[format_index]) {
                input_index++;
                format_index++;
            } else {
                // Literal character doesn't match
                break;
            }
        }
    }
    
    va_end(args);
    return parsed_items;
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
    // uint64_t millis_elapsed = getMillisElapsed();
    uint64_t millis_condition = millis;
    SleepCondition condition = { .millis = millis_condition };
    // log_decimal("MILLIS ELAPSED: ", millis_elapsed);
    // log_decimal("SLEEPING FOR: ", millis);
    // log_decimal("MILLIS CONDITION: ", millis_condition);
    waitForEvent(SLEEP_EVENT, NULL, (void *)&condition);
    // syscall(SLEEP_SYSCALL, millis, 0, 0, 0, 0);
}

uint64_t getMillisElapsed(){
    uint64_t millis;
    syscall(GET_MILLIS_ELAPSED_SYSCALL, (uint64_t)&millis, 0, 0, 0, 0);
    return millis;
}

void seedRandom(uint64_t seed){
    randmon_seed = (uint32_t)(seed & 0xFFFFFFFF);
}

uint32_t randInt(uint32_t min, uint32_t max){
    randmon_seed = randmon_seed * 1664525 + 1013904223;
    return (randmon_seed % (max - min + 1)) + min;
}

// BACKWARDS COMPATIBILITY
char get_char_from_stdin(){
    char c;
    readStdin(&c, 1);
    return c;
}