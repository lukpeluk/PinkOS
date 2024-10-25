/* PinkOS.c */
#include <stdint.h>
#include <stdint.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2);

// draft of how the buffer should be (to store terminal input)
// it's an array of strints of fixed length, so that each enter key press will be stored in a new string
// when the buffer is full, the oldest string will be removed
// an index for the current string should be maintained
// also an index for the current position in the string should be maintained
// and an index for the oldest string should be maintained (to stop reading after that)
// the strings are -1 terminated
// the buffer should be printed in reverse order, so that the most recent string is printed first

#define BUFFER_SIZE 100
#define STRING_SIZE 500 // 99 usable characters and the -1 termination

char buffer[BUFFER_SIZE][STRING_SIZE] = {-1};
int current_string = 0;
int current_position = 0;
int oldest_string = 0;

int scroll = 0; // indica qué línea es la que está en la parte superior de la pantalla

// saves the key to the buffer
// 0 for success, -1 if save was not performed (string size exceeded for example)
//
// me harté de escribir en inglés, perdonenme en esta descripción:
// el buffer tiene dos índices, uno para el string actual y uno para el más viejo guardado.
// la idea es que al llegar al final del arreglo, vuelve a empezar, sobreescribiendo el valor más viejo
// cuando el buffer de escritura (el del str actual) llega al buffer de lectura (el del más viejo), se mueve el buffer de lectura
// cuando quiero leer toda la terminal con su historial, empiezo desde el buffer de lectura y termino en el buffer de escritura
// (obviamnte también al leer tengo que hacer un wrap around cuando llego al final del buffer)
int save_to_buffer(char key){
	// if enter key is pressed, move to the next string
	if(key == 37){
		// move to the next string, if the end is reached, wrap around to the oldest string
		current_string++;
		if(current_string == BUFFER_SIZE){
			current_string = 0;
		}

		// if the buffer is full, the oldest string gets overwritten, move the oldest string index to the next string
		// also wrap around if the end is reached
		if(current_string == oldest_string){
			oldest_string++;
			if(oldest_string == BUFFER_SIZE){
				oldest_string = 0;
			}
		}

		// reset the current position and terminate the string with -1
		current_position = 0;
		buffer[current_string][current_position] = -1;
	}
	// now check if the key is printable
	// if it is, save it to the buffer and print it to the screen
	else if(key >= 0 && key <= 36){
		// checks if the buffer is full, if it is, key presses are ignored
		if(current_position == STRING_SIZE - 1)
			return -1;

		// save key to buffer
		buffer[current_string][current_position] = key;
		current_position++;
		buffer[current_string][current_position] = -1;
	}

	return 0;
}

void key_handler(char key){
	int result = save_to_buffer(key);

	// if the string limit is reached, it simply does nothing until the enter key is pressed or you delete characters
	if(result == -1)
		return;

	// print key to screen
	syscall(1, key, 0x00df8090);
}

int main() {
	// set key handler, function pointer
	syscall(2, key_handler, 0);
}
