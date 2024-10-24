/* sampleCodeModule.c */
#include <stdint.h>
#include <stdint.h>

char * v = (char*)0xB8000 + 79 * 2;

static int var1 = 0;
static int var2 = 0;

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2);


void key_handler(char key){
	syscall(1, key, 0x00df8090);
}

int main() {

	// print char, caracter, color
	syscall(1, 0, 0x00df8090);

	// set key handler, function pointer
	syscall(2, key_handler, 0);


	/*
	//All the following code may be removed 
	*v = 'X';
	*(v+1) = 0x74;

	//Test if BSS is properly set up
	if (var1 == 0 && var2 == 0)
		return 0xDEADC0DE;

	return 0xDEADBEEF;
	*/
}
