/* PinkOS.c */
#include <stdint.h>
#include <stdint.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

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
		// and automatically scroll if a visible line is overwritten
		if(current_string == oldest_string){
			oldest_string++;
			if(oldest_string == BUFFER_SIZE){
				oldest_string = 0;
			}
			// TODO: ver si este código está bien acá o es más prolijo abstraer la lógica del scroll
			if(oldest_string == scroll){
				scroll++;
				if(scroll == BUFFER_SIZE){
					scroll = 0;
					redraw();
				}
			}
		}

		// reset the current position and terminate the string with -1
		current_position = 0;
		buffer[current_string][current_position] = -1;
	}
	// check if the key is printable, if it is it's saved to the buffer
	else if(key >= 0 && key <= 36){
		// checks if the buffer is full, if it is, key presses are ignored
		if(current_position == STRING_SIZE - 1)
			return -1;

		// save key to buffer
		buffer[current_string][current_position] = key;
		current_position++;
		buffer[current_string][current_position] = -1;
	}
	// backspace
	else if(key == 38){
		if(current_position > 0){
			current_position--;
			buffer[current_string][current_position] = -1;
		}
	}
	else{
		// unsupported key
		return -1;
	}

	return 0;
}

void redraw(){
	// clear screen
	syscall(2, 0x00000000, 0, 0, 0, 0);

	// print the buffer from the scroll position to the current string

	int i = scroll - 1;
	do {
		i++;

		// wraps arround
		if(i == BUFFER_SIZE){
			i = 0;
		}

		// print the string
		for(int j = 0; buffer[i][j] != -1; j++){
			syscall(1, buffer[i][j], 0x00df8090, 0x00000000, 0, 0);
		}

		// print a new line (except in the last string)
		if(i != current_string)
			syscall(1, 37, 0x00df8090, 0x00000000, 0, 0);

	} while (i != current_string);


}

void clear_buffer(){
	// clear the buffer
	for(int i = 0; i < BUFFER_SIZE; i++){
		buffer[i][0] = -1;
	}
	current_position = 0;
	current_string = 0;
	oldest_string = 0;
	scroll = 0;
}

void key_handler(char key){

	// for testing purposes:
	// <1> will scroll the current line to the top, 
	// <2> will scroll up the buffer one line,
	// <3> will scroll the buffer all the way up,
	// <4> will redraw the screen
	// <5> will clear the buffer and the screen
	if(key == 1){
		scroll = current_string;
		redraw();
		return;
	}
	if(key == 2){
		if(scroll == oldest_string)
			return;

		if(scroll == 0)
			scroll = BUFFER_SIZE;
		scroll = scroll - 1;

		redraw();
		return;
	}
	if(key == 3){
		scroll = oldest_string;
		redraw();
		return;
	}
	if(key == 4){
		redraw();
		return;
	}
	if(key == 5){
		clear_buffer();
		redraw();
		return;
	}


	// dont_draw evita que se dibuje el caracter si se presiona delete en la primera posición
	// (si no estaría borrando la línea anterior)
	int dont_draw = 0;
	if(key == 38 && current_position == 0){
		dont_draw = 1;	
	}

	int result = save_to_buffer(key);

	// if the string limit is reached or the key is unsupported, it simply does nothing
	if(result == -1 || dont_draw)
		return;

	// print key to screen
	syscall(1, key, 0x00df8090, 0x00000000, 0, 0);
}

int main() {
	// set key handler, function pointer
	syscall(3, key_handler, 0, 0, 0, 0);
}
