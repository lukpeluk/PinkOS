#include <libs/serialLib.h>
#include <syscalls/syscallCodes.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2);

void make_ethereal_request(char * request, EtherPinkResponse * response){
    syscall(MAKE_ETHEREAL_REQUEST_SYSCALL, (uint64_t)request, (uint64_t)response);
}

void log_to_serial(char * message) {
    syscall(LOG_TO_SERIAL_SYSCALL, (uint64_t)message, 0);
}