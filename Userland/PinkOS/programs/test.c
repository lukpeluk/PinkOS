#include <programs.h>
#include <stdpink.h>
#include <stdint.h>

extern void make_0x0_exception();
extern void make_0x6_exception();

void test_main(unsigned char * args){
    if (strcmp(args, "0") == 0){    // Zero Division Exception
        printf((unsigned char *)"Testing Zero Division Exception...\n");
        make_0x0_exception();
    }else if (strcmp(args, "6") == 0){ // Invalid Opcode Exception
        printf((unsigned char *)"Testing Invalid Opcode Exception...\n");
        make_0x6_exception();
    }else{
        printf((unsigned char *)"Invalid argument. Options are: 0, 6\n");
    }

    return;
}