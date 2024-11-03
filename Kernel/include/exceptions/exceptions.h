#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>
#define ZERO_EXCEPTION_ID 0x0
#define INVALID_OPCODE_EXCEPTION_ID 0x6


typedef struct {
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t cri_rip, cri_rsp, cri_rflags;
} BackupRegisters;



#endif