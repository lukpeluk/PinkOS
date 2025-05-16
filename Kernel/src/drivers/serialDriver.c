#include <drivers/serialDriver.h>
#include <drivers/videoDriver.h>

#define RAW_DATA_ADDRESS 0x600000
#define MAX_RAW_DATA_ADDRESS 0xFFFFFF // para pensar

static EtherPinkResponse * clientResponse;
static char * current = 0;
// static uint32_t total_size = 300*300*4;
static uint32_t total_size = 4728;

void process_serial(char c){
    if(current && clientResponse){
        *current = c;
        current++;
        clientResponse->size++;
        // process_header();

        // if( c == 0 ){
        if (clientResponse->size == total_size){
            current = 0;
            clientResponse->code = SUCCESS;
            clientResponse->raw_data = (char *)RAW_DATA_ADDRESS;
            clientResponse = 0;
            // process response
        }
    }
}

void make_ethereal_request(char * request, EtherPinkResponse * response){
    clientResponse = response;
    clientResponse->code = 0;
    clientResponse->type = 0;
    clientResponse->size = 0;
    clientResponse->raw_data = 0;

    current = (char *) RAW_DATA_ADDRESS;
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
        current = (char *) RAW_DATA_ADDRESS; 
    }
}