#include <drivers/serialDriver.h>
#include <drivers/videoDriver.h>

#define RAW_DATA_ADDRESS 0x600000
#define MAX_RAW_DATA_ADDRESS 0xFFFFFF // para pensar

static EtherPinkResponse * clientResponse;
static unsigned char * current = 0;
static uint32_t total_size = -1;

void process_serial(unsigned char c){
    if(current && clientResponse){
        *current = c;
        current++;
        clientResponse->size++;
        // process_header();

        if( c == 0 ){
            current = 0;
            clientResponse->code = SUCCESS;
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

    current = (unsigned char *) RAW_DATA_ADDRESS;
    while(*request){
        write_serial(*request);
        request++;
    } 
    // while(clientResponse){
    //     process_serial(read_serial());
    // }   
}

void process_header(){
    if(clientResponse->size > HEADER_SIZE && clientResponse->code == NO_DATA_YET){
        clientResponse->code = LOADING;
        clientResponse->type = *(uint8_t*)(RAW_DATA_ADDRESS + 16);
        clientResponse->size = 0;
        total_size = *(uint32_t*)(RAW_DATA_ADDRESS + 32);
        current = (unsigned char *) RAW_DATA_ADDRESS; 
    }
}