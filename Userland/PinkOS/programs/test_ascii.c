#include <libs/stdpink.h>


void test_ascii_main(char *args){
    print((char *)"Testing ascii:");
    char i = 0;
    while(1){
        printf((char *)"Char %d: ", i);
        putChar(i);
        putChar('\n');
        i++;
        if(0) break;
    }
}