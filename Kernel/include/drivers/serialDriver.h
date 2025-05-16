#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <stdint.h>

// CONTENT TYPES
#define PLAIN_TEXT 0
#define BITMAP 1
#define AUDIO 2

// RESPONSE CODES
#define NO_DATA_YET 0
#define LOADING 1
#define SUCCESS 2
#define ERROR 3

// RESPONSE TYPES
#define FIXED_SIZE 0
#define ASCII_STREAM 1


#define HEADER_SIZE 8 // 2 byte for response code, 1 byte for content type, 1 byte for response type and 4 bytes for data size


typedef struct EtherPinkResponse{
    uint16_t code, type;
    uint64_t size;
    char *raw_data;
} EtherPinkResponse;


extern void init_setial();
extern void test_serial();

extern char read_serial();
extern void write_serial(char c);

void process_serial(char c);

#endif