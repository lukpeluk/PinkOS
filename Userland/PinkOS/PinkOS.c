/* PinkOS.c */
#include <stdint.h>
#include <stdint.h>
#include <syscallCodes.h>
#include <programs.h>
#include <environmentApiEndpoints.h>

#define PREV_STRING current_string > 0 ? current_string - 1 : BUFFER_SIZE - 1
#define ADVANCE_INDEX(index) index = (index + 1 == BUFFER_SIZE) ? 0 : index + 1;

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

// draft of how the buffer should be (to store terminal input)
// it's an array of strints of fixed length, so that each enter key press will be stored in a new string
// when the buffer is full, the oldest string will be removed
// an index for the current string should be maintained
// also an index for the current position in the string should be maintained
// and an index for the oldest string should be maintained (to stop reading after that)
// the strings are null terminated
// the buffer should be printed in reverse order, so that the most recent string is printed first

int shell_active = 1; // TODO: pensar nombre mejor (?)

#define BUFFER_SIZE 100
#define STRING_SIZE 200 // 199 usable characters and the null termination

static const char * command_not_found_msg = "Command not found\n";
static const char * default_prompt = " > ";

char buffer[BUFFER_SIZE][STRING_SIZE] = {0};
uint8_t is_input[BUFFER_SIZE] = {0}; // 1 if the string is an input, 0 if it's an output
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
int save_char_to_buffer(char key){
	// if enter key is pressed, move to the next string
	if(key == '\n'){
		ADVANCE_INDEX(current_string) // move to the next string

		// if the buffer is full, the oldest string gets overwritten, move the oldest string index to the next string
		// and automatically scroll if a visible line is overwritten
		if(current_string == oldest_string){
			ADVANCE_INDEX(oldest_string)

			// TODO: ver si este código está bien acá o es más prolijo abstraer la lógica del scroll
			if(oldest_string == scroll){
				ADVANCE_INDEX(scroll)
			}
		}

		// reset the current position and terminate the string with 0
		current_position = 0;
		buffer[current_string][current_position] = 0;
		is_input[current_string] = 0;  // lines are not input until assigned as such by newPrompt()
	}
	// check if the key is printable, if it is it's saved to the buffer
	else if(key >= 32 && key <= 126){
		// checks if the buffer is full, if it is, key presses are ignored
		if(current_position == STRING_SIZE - 1)
			return -1;

		// save key to buffer
		buffer[current_string][current_position] = key;
		current_position++;
		buffer[current_string][current_position] = 0;
	}
	// backspace
	else if(key == 8){
		if(current_position > 0){
			current_position--;
			buffer[current_string][current_position] = 0;
		}
	}
	else{
		// unsupported key
		return -1;
	}

	return 0;
}

int save_str_to_buffer(char * string){
	for(int i = 0; string[i] != 0; i++){
		save_char_to_buffer(string[i]);
	}	
}

void redraw(){
	// clear screen
	syscall(CLEAR_SCREEN_SYSCALL, 0x00000000, 0, 0, 0, 0);

	// print the buffer from the scroll position to the current string

	int i = scroll ? scroll - 1 : BUFFER_SIZE - 1;
	do {
		ADVANCE_INDEX(i)

		if(is_input[i] == 1){
			syscall(DRAW_STRING_SYSCALL, default_prompt, 0x00df8090, 0x00000000, 0, 0);
		}
		syscall(DRAW_STRING_SYSCALL, buffer[i], 0x00df8090, 0x00000000, 0, 0);

		// print a new line (except in the last string)
		if(i != current_string)
			syscall(DRAW_CHAR_SYSCALL, '\n', 0x00df8090, 0x00000000, 0, 0);

	} while (i != current_string);
}

void clear_buffer(){
	// clear the buffer
	for(int i = 0; i < BUFFER_SIZE; i++){
		buffer[i][0] = 0;
	}
	current_position = 0;
	current_string = 0;
	oldest_string = 0;
	scroll = 0;
}

// prints and saves to the buffer
void print_char_to_console(char * key){
	// presionar delete en la primera posición no hace nada
	if(key == 8 && current_position == 0){
		return;
	}

	int result = save_char_to_buffer(key);

	// if the string limit is reached or the key is unsupported, it simply does nothing
	if(result == -1)
		return;

	// print key to screen
	syscall(DRAW_CHAR_SYSCALL, key, 0x00df8090, 0x00000000, 0, 0);
}

// prints and saves to the buffer
void print_str_to_console(char * string){
	for(int i = 0; string[i] != 0; i++){
		print_char_to_console(string[i]);
	}
}

void execute_program(int input_line){
		// get the program name
		char program_name[STRING_SIZE];
		int i = 0;
		for(; buffer[input_line][i] != ' ' && buffer[input_line][i] != 0; i++){
			program_name[i] = buffer[input_line][i];
		}
		program_name[i] = 0;

		if(buffer[input_line][i] == ' ')
			i++;

		// get the arguments
		char arguments[STRING_SIZE];
		int j = 0;
		for(; buffer[input_line][i] != 0; i++, j++){
			arguments[j] = buffer[input_line][i];
		}
		arguments[j] = 0;

		// get the program entry point
		Program * program = get_program_entry(program_name);

		// if the program is not found, print an error message
		if(program == 0){
			// "Command not found"
			save_str_to_buffer(command_not_found_msg);
			syscall(DRAW_STRING_SYSCALL, command_not_found_msg, 0x00df8090, 0x00000000, 0, 0);
			newPrompt();
		}
		// if the program is found, execute it
		else{
			// Tiene que llamar a un syscall que ejecute el programa
			syscall(RUN_PROGRAM_SYSCALL, program, arguments, 0, 0, 0);
		}
}


void api_handler(uint64_t endpoint_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
	switch (endpoint_id)
	{
	case PRINT_STRING_ENDPOINT:
		save_str_to_buffer((char *)arg1);
		syscall(DRAW_STRING_SYSCALL, (char *)arg1, 0x00df8090, 0x00000000, 0, 0);
		break;
	
	default:
		break;
	}
}

void key_handler(char key){

	// for testing purposes:
	// <1> will scroll the current line to the top, 
	// <2> will scroll up the buffer one line,
	// <3> will scroll the buffer all the way up,
	// <4> will redraw the screen
	// <5> will clear the buffer and the screen
	if(key == '1'){
		scroll = current_string;
		redraw();
		return;
	}
	if(key == '2'){
		if(scroll == oldest_string)
			return;

		if(scroll == 0)
			scroll = BUFFER_SIZE;
		scroll = scroll - 1;

		redraw();
		return;
	}
	if(key == '3'){
		scroll = oldest_string;
		redraw();
		return;
	}
	if(key == '4'){
		redraw();
		return;
	}
	if(key == '5'){
		clear_buffer();
		redraw();
		return;
	}


	print_char_to_console(key);
	if(key == '\n' && shell_active){
		execute_program(PREV_STRING);
	}
}

// configures the current line as a prompt, and prints a graphical indicator of that
void newPrompt(){
	// print the prompt
	is_input[current_string] = 1;
	syscall(DRAW_STRING_SYSCALL, default_prompt, 0x00df8090, 0x00000000, 0, 0);
}

void restoreContext(){
	redraw();
	print_char_to_console('\n');
	newPrompt();
	shell_active = 1;
}


int main() {
	newPrompt();
	shell_active = 1;

	// Setea todos los handlers, para quedar corriendo "en el fondo"
	syscall(SET_HANDLER_SYSCALL, 0, key_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 3, restoreContext, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 4, api_handler, 0, 0, 0);
}
