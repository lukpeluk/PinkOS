/* PinkOS.c */
#include <stdint.h>
#include <stdint.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2);


void key_handler(char key){
	syscall(1, key, 0x00df8090);
}

int main() {
	// set key handler, function pointer
	syscall(2, key_handler, 0);
}
