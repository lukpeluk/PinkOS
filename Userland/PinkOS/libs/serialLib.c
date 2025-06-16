#include <libs/serialLib.h>
#include <syscalls/syscallCodes.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2);

void make_ethereal_request(char * request, EtherPinkResponse * response){
    syscall(MAKE_ETHEREAL_REQUEST_SYSCALL, (uint64_t)request, (uint64_t)response);
}

void log_to_serial(const char * message) {
    syscall(LOG_TO_SERIAL_SYSCALL, (uint64_t)message, 0);
}

void log_decimal(const char * message, uint64_t number) {
    syscall(LOG_DECIMAL_TO_SERIAL_SYSCALL, (uint64_t)message, number);
}

void log_hex(const char * message, uint64_t number) {
    syscall(LOG_HEX_TO_SERIAL_SYSCALL, (uint64_t)message, number);
}