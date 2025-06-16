#include <libs/stdpink.h>
#include <syscalls/syscall.h>

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

Pid runProgram(Program * program, char * args, Priority priority, IO_Files * io_files, int nohup) {
    return (Pid)syscall(RUN_PROGRAM_SYSCALL, (uint64_t)program, (uint64_t)args, (uint64_t)priority, (uint64_t)io_files, (uint64_t)nohup);
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
    return (int)syscall(READ_STDIN, (uint64_t)buffer, (uint64_t)size, 0, 0, 0);
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

uint64_t mkFile(char * path, FileType type, uint32_t size, FilePermissions permissions) {
    return syscall(MK_FILE_SYSCALL, (uint64_t)path, (uint64_t)type, (uint64_t)size, (uint64_t)&permissions, 0);
}

int rmFile(char * path) {
    return (int)syscall(RM_FILE_SYSCALL, (uint64_t)path, 0, 0, 0, 0);
}

uint64_t openFile(char * path, Pid pid, FileAction action, FileType type) {
    return syscall(OPEN_FILE_SYSCALL, (uint64_t)path, (uint64_t)pid, (uint64_t)action, (uint64_t)type, 0);
}

int closeFile(uint64_t id, Pid pid) {
    return (int)syscall(CLOSE_FILE_SYSCALL, id, (uint64_t)pid, 0, 0, 0);
}

int closeFifoForWriting(uint64_t id) {
    return (int)syscall(CLOSE_FIFO_FOR_WRITING_SYSCALL, id, 0, 0, 0, 0);
}

int readRaw(uint64_t id, void * buffer, uint32_t size, uint32_t offset) {
    int result;
    syscall(READ_RAW_FILE_SYSCALL, id, (uint64_t)buffer, (uint64_t)size, (uint64_t)offset, (uint64_t)&result);
    return result;
}

int writeRaw(uint64_t id, void * buffer, uint32_t size, uint32_t offset) {
    int result;
    syscall(WRITE_RAW_FILE_SYSCALL, id, (uint64_t)buffer, (uint64_t)size, (uint64_t)offset, (uint64_t)&result);
    return result;
}

int readFifo(uint64_t id, void * buffer, uint32_t size) {
    int result;
    syscall(READ_FIFO_FILE_SYSCALL, id, (uint64_t)buffer, (uint64_t)size, (uint64_t)&result, 0);
    return result;
}

int writeFifo(uint64_t id, void * buffer, uint32_t size) {
    int result;
    syscall(WRITE_FIFO_FILE_SYSCALL, id, (uint64_t)buffer, (uint64_t)size, (uint64_t)&result, 0);
    return result;
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

int setFilePermissions(uint64_t id, Pid pid, FilePermissions permissions) {
    int result;
    syscall(SET_FILE_PERMISSIONS_SYSCALL, id, (uint64_t)pid, (uint64_t)&permissions, (uint64_t)&result, 0);
    return result;
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
    writeStdout(string, strlen(string));
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
                    } else {
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
                    writeStderr(invalid_format_message, strlen(invalid_format_message));
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