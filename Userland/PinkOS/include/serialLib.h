#ifndef SERIAL_LIB_H
#define SERIAL_LIB_H

#include <stdint.h>

typedef struct EtherPinkResponse{
    uint16_t code, type;
    uint64_t size;
    char *raw_data;
} EtherPinkResponse;


void make_ethereal_request(char * request, EtherPinkResponse * response);



#endif