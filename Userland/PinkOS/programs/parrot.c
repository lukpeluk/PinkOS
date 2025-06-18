#include <programs.h>
#include <syscalls/syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <libs/stdpink.h>
#include <stdin.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

#define BUFFER_SIZE 200

void parrot_main(char * args){
    char buffer[BUFFER_SIZE];
    char c;
    int read;

    while (1){
        // syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, (uint64_t) " - ", 0, 0, 0);
        print(" - ");

        int i = 0;
        do{
            read = readStdin(&c, 1);
            if(i < BUFFER_SIZE - 1 && c && read){
                buffer[i] = c;
                i++;
            }
        } while(c != '\n' && read > 0); 
        
        buffer[i] = 0;
        // syscall(USER_ENVIRONMENT_API_SYSCALL, PRINT_STRING_ENDPOINT, (uint64_t) buffer, 0, 0, 0);
        print(buffer);
    }
}