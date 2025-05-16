#include <serialLib.h>
#include <syscallCodes.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2);

void make_ethereal_request(char * request, EtherPinkResponse * response){
    syscall(MAKE_ETHEREAL_REQUEST_SYSCALL, (uint64_t)request, (uint64_t)response);
}