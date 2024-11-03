#include <programs.h>
#include <stdpink.h>
#include <stdint.h>

extern void make_0x0_exception();
extern void make_0x6_exception();

void test_main(char * args){
    if (strcmp(args, "0") == 0){    // Zero Division Exception
        printf("Hola\n");
        make_0x0_exception();
    }else if (strcmp(args, "6") == 0){ // Invalid Opcode Exception
        make_0x6_exception();
    }

    return;
}