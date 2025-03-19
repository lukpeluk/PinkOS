#include <programs.h>
#include <serialLib.h>
#include <stdpink.h>


void apt_install_main(unsigned char * args){
    unsigned char buffer[200];
    unsigned char c;
    EtherPinkResponse response;

    buffer[0] = 'a';
    buffer[1] = 'p';
    buffer[2] = 't';
    buffer[3] = ' ';
    buffer[4] = 'i';
    buffer[5] = 'n';
    buffer[6] = 's';
    buffer[7] = 't';
    buffer[8] = 'a';
    buffer[9] = 'l';
    buffer[10] = 'l';
    buffer[11] = ' ';
    buffer[12] = ' ';
    buffer[13] = 0;

    while (1){
        printf((unsigned char *)" -> ");

        int i = 13;
        do{
            c = getChar();
            if(i < 200 - 1 && c){
                buffer[i] = c;
                i++;
            }
        } while(c != '\n');
        
        buffer[i] = 0;
        make_ethereal_request(buffer, &response);
        // print(buffer);

        while(response.code == 0){
            // wait for response
            sleep(100);
        }
        ((void (*)())response.raw_data)();
        for (int i = 0; i < 4656; i++){
            putChar(response.raw_data[i]);
        }
        print("Done");

        // print the response in hexa
        // for (int i = 0; i < response.size; i++){
        //     putChar(response.raw_data[i]);
        // }
        
    }
}