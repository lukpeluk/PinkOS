#ifndef SYSCALLDISPATCHER_H
#define SYSCALLDISPATCHER_H
#include <stdint.h>

uint64_t syscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

#endif