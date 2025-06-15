#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <syscalls/syscallCodes.h>

extern uint64_t syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);


#endif