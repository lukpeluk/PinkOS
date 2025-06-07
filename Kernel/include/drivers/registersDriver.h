#ifndef REGISTERS_DRIVER_H
#define REGISTERS_DRIVER_H

#include <stdint.h>

typedef struct {
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
} Registers;

typedef struct {
	Registers registers;
	uint64_t cri_rip, cri_rsp, cri_rflags;
} BackupRegisters;

void saveRegisters();

BackupRegisters * getBackupRegisters();





#endif