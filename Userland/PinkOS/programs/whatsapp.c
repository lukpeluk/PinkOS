#include <libs/serialLib.h>
#include <programs.h>
#include <syscalls/syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <libs/stdpink.h>

#define BUFFER_SIZE 200


void whatsapp_main(char * args){
    char buffer[BUFFER_SIZE];
    char c;
    EtherPinkResponse response;

    while (1){
        printf((char *)" -> ");

        int i = 0;
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

        response.raw_data[response.size] = 0;
        print(response.raw_data);
    }
}


