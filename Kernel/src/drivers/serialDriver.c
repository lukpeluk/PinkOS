#include <drivers/serialDriver.h>

#define RAW_DATA_ADDRESS 0x600000
#define MAX_RAW_DATA_ADDRESS 0xFFFFFF // para pensar

static EtherPinkResponse * clientResponse;
static unsigned char * current = 0;

void process_serial(unsigned char c){
    if(current && clientResponse){
        *current = c;
        current++;
        clientResponse->size++;

        if( clientResponse->size == 640*640*4){
            current = 0;
            clientResponse->code = 1;
            clientResponse->type = 1;
            clientResponse->raw_data = (unsigned char *)RAW_DATA_ADDRESS;
            clientResponse = 0;
            // process response
        }
    }
}

void make_ethereal_request(unsigned char * request, EtherPinkResponse * response){
    clientResponse = response;
    clientResponse->code = 0;
    clientResponse->type = 0;
    clientResponse->size = 0;
    clientResponse->raw_data = 0;

    current = RAW_DATA_ADDRESS;
    while(*request){
        write_serial(*request);
        request++;
    } 
    // while(clientResponse){
    //     process_serial(read_serial());
    // }   
}

void process_header(){
    while (clientResponse->size < 8){
        process_serial(read_serial());
    }
}