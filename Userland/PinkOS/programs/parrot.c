#include <programs.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdin.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

#define BUFFER_SIZE 200

void parrot_main(unsigned char * args){
    unsigned char buffer[BUFFER_SIZE];
    unsigned char c;

    while (1){
        syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, (uint64_t) " - ", 0, 0, 0);

        int i = 0;
        do{
            c = get_char_from_stdin();
            if(i < BUFFER_SIZE - 1 && c){
                buffer[i] = c;
                i++;
            }
        } while(c != '\n');
        
        buffer[i] = 0;
        syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, (uint64_t) buffer, 0, 0, 0);
    }
}