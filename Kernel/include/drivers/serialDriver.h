#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <stdint.h>

typedef struct EtherPinkResponse{
    uint16_t code, type;
    uint64_t size;
    unsigned char *raw_data;
} EtherPinkResponse;


extern void init_setial();
extern void test_serial();

extern char read_serial();
extern void write_serial(unsigned char c);

void process_serial(unsigned char c);

#endif