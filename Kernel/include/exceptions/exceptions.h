#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>
#define ZERO_EXCEPTION_ID 0x0
#define INVALID_OPCODE_EXCEPTION_ID 0x6

typedef struct {
    uint64_t exception_id; // Exception ID
    uint64_t error_code;   // Error code, if applicable
    uint64_t rip;          // Instruction pointer at the time of the exception
    uint64_t rsp;          // Stack pointer at the time of the exception
    uint64_t rflags;       // Flags register at the time of the exception
} Exception;


#endif