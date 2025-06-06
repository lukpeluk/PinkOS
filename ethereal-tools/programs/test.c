
typedef unsigned long uint64_t;

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

int main(int argc, char const *argv[])
{
    syscall(1051, 1, 0, 0, 0, 0);
    return 0;
}
