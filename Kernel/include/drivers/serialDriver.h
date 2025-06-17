#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <stdint.h>
#include <lib.h>

// CONTENT TYPES
#define PLAIN_TEXT 0
#define BITMAP 1
#define AUDIO 2

// RESPONSE CODES
#define NO_DATA_YET 0
#define LOADING 1
#define SUCCESS 2
#define ERROR 3
#define GETTING_HEADERS 4
#define SERIAL_OCCUPIED 5

// RESPONSE TYPES
#define FIXED_SIZE 0
#define ASCII_STREAM 1


#define HEADER_SIZE 14 // 2 bytes for response code, 2 bytes for content type, 2 bytes for response type, and 8 bytes for data size


typedef struct EtherPinkResponse{
    uint16_t code, content_type, response_type;
    uint64_t size;
    char *raw_data;
} EtherPinkResponse;


void process_serial(char c);
void make_ethereal_request(char * request, EtherPinkResponse * response); 

void log_to_serial(char * message);


void log_hex(char* prefix, uint64_t value);
void log_decimal(char* prefix, uint64_t value);
void log_string(char* prefix, char* str);
void log_to_serial(char* message);

// Memory debugging functions
void mem_register_sector(uint64_t start_addr, uint64_t end_addr, char* tag);
void mem_log_address(uint64_t address, char* name);
void mem_free_sector(uint64_t start_addr);
void mem_list_sectors(); // Lista todos los sectores registrados

// Macros útiles para debugging de memoria
#define MEMREG(start, end, tag) mem_register_sector((uint64_t)(start), (uint64_t)(end), tag)
#define MEMLOG(addr, name) mem_log_address((uint64_t)(addr), name)
#define MEMFREE(start) mem_free_sector((uint64_t)(start))
#define MEMLIST() mem_list_sectors()

// Macros para logging de punteros y variables
#define LOG_PTR(name, ptr) mem_log_address((uint64_t)(ptr), name)
#define LOG_VAR_ADDR(var) mem_log_address((uint64_t)(&var), #var)


// asm, low level
extern void init_serial();
extern void test_serial();

extern char read_serial();
extern void write_serial(char c);


#endif