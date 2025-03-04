#include <serialLib.h>
#include <programs.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdpink.h>

#define BUFFER_SIZE 200


void chatgpt_main(unsigned char * args){
    unsigned char buffer[BUFFER_SIZE];
    unsigned char c;
    EtherPinkResponse response;

    buffer[0] = 'c';
    buffer[1] = 'h';
    buffer[2] = 'a';
    buffer[3] = 't';
    buffer[4] = 'g';
    buffer[5] = 'p';
    buffer[6] = 't';
    buffer[7] = ' ';

    while (1){
        printf((unsigned char *)" -> ");

        int i = 8;
        do{
            c = getChar();
            if(i < BUFFER_SIZE - 1 && c){
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

        print(response.raw_data);
    }
}


