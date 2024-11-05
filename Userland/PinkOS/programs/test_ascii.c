#include <stdpink.h>


void test_ascii_main(unsigned char *args){
    print("Testing ascii:");
    unsigned char i = 0;
    while(1){
        printf("Char %d: ", i);
        putChar(i);
        putChar('\n');
        i++;
        if(0) break;
    }
}