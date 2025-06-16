#ifndef SERIAL_LIB_H
#define SERIAL_LIB_H

#include <stdint.h>

typedef struct EtherPinkResponse{
    uint16_t code, type;
    uint64_t size;
    char *raw_data;
} EtherPinkResponse;


void make_ethereal_request(char * request, EtherPinkResponse * response);

void log_to_serial(const char * message); 
void log_decimal(const char * message, uint64_t value);
void log_hex(const char * message, uint64_t value);



#endif