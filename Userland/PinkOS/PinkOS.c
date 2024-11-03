/* PinkOS.c */
#include <stdint.h>
#include <stdint.h>
#include <syscallCodes.h>
#include <programs.h>
#include <permissions.h>
#include <environmentApiEndpoints.h>
#include <ascii.h>
#include <stdpink.h>	
#include <graphicsLib.h>
#include <keyboard.h>

#define PREV_STRING current_string > 0 ? current_string - 1 : BUFFER_SIZE - 1
#define ADVANCE_INDEX(index) index = (index + 1) % BUFFER_SIZE;
#define IS_PRINTABLE(ascii) ((ascii >= 32 && ascii <= 126) || ascii == '\n' || ascii == '\b')

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
extern void * get_stack_pointer();
extern void _hlt();


typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t day_of_week;
} RTC_Time;

typedef struct {
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
} Registers;

typedef struct {
	Registers registers;
	uint64_t cri_rip, cri_rsp, cri_rflags;
} BackupRegisters;


int running_program = 0; // 0 if no program is running (besides the shell itself, ofc)
int graphics_mode = 0; // 0 for CLI, 1 for GUI

#define BUFFER_SIZE 500
#define STRING_SIZE 200 // 199 usable characters and the null termination
#define KEY_REPEAT_ENABLED 1    // 0 for disabling key repeat

static char time_str[9] = "00:00:00";
static Point time_position = {950, 0};
static Point logo_position = {10, 0};

#define TIME_PADDING 10

static const char * command_not_found_msg = "Command not found\n";
static const char * default_prompt = " > ";

char buffer[BUFFER_SIZE][STRING_SIZE] = {0};
uint8_t is_input[BUFFER_SIZE] = {0}; // 1 if the string is an input, 0 if it's an output
int current_string = 0;
int current_position = 0;
int oldest_string = 0;

unsigned char stdin_buffer[STRING_SIZE] = {0};
int stdin_write_position = 0;
int stdin_read_position = 0;

unsigned char get_char_from_stdin(){
	if(stdin_write_position == stdin_read_position){
		return 0;
	}

	unsigned char c = stdin_buffer[stdin_read_position];
	stdin_read_position = (stdin_read_position + 1) % STRING_SIZE;
	return c;
}

void add_char_to_stdin(unsigned char c){
	stdin_buffer[stdin_write_position] = c;
	stdin_write_position = (stdin_write_position + 1) % STRING_SIZE;
	if(stdin_write_position == stdin_read_position){
		stdin_read_position = (stdin_read_position + 1) % STRING_SIZE;
	}
}

void clear_stdin(){
	stdin_read_position = stdin_write_position;
}

int scroll = 0; // indica qué línea es la que está en la parte superior de la pantalla

// The buffer is an array of strings of fixed length, so that each enter key press will be stored in a new string
// when the buffer is full, the oldest string will be removed
// an index for the current string should be maintained
// also an index for the current position in the string should be maintained
// and an index for the oldest string should be maintained (to stop reading after that)
// the strings are null terminated
// the buffer should be printed in reverse order, so that the most recent string is printed first

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
			// igual la única forma de que esto pase es que el buffer sea más chico que la pantalla
			if(oldest_string == scroll){
				ADVANCE_INDEX(scroll)
			}
		}

		// reset the current position and terminate the string with 0
		current_position = 0;
		buffer[current_string][current_position] = 0;
		is_input[current_string] = 0;  // lines are not input until assigned as such by newPrompt()
	}
	// backspace
	else if(key == 8){
		if(current_position > 0){
			current_position--;
			buffer[current_string][current_position] = 0;
		}
	}
	else if(IS_PRINTABLE(key)){
		// checks if the buffer is full, if it is, key presses are ignored
		if(current_position == STRING_SIZE - 1)
			return -1;

		// save key to buffer
		buffer[current_string][current_position] = key;
		current_position++;
		buffer[current_string][current_position] = 0;
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

int save_number_to_buffer(uint64_t number){
	char buffer[12];
	int i = 0;
	if(number == 0){
		buffer[i++] = '0';
	}
	else{
		while(number > 0){
			buffer[i++] = number % 10 + '0';
			number /= 10;
		}
	}
	buffer[i] = 0;

	for(int j = i - 1; j >= 0; j--){
		save_char_to_buffer(buffer[j]);
	}
}

void redraw(){
	// clear screen
	syscall(CLEAR_SCREEN_SYSCALL, 0x00000000, 0, 0, 0, 0);
	draw_status_bar();
	syscall(SET_CURSOR_LINE_SYSCALL, 1, 0, 0, 0, 0);  // evita dibujar la status bar

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
			syscall(DRAW_CHAR_SYSCALL, '\n', 0x00df8090, 0x00000000, 1, 0);

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

	// si el caracter haría wrappeo, scrolleo una línea
	

	int result = save_char_to_buffer(key);

	// if the string limit is reached or the key is unsupported, it simply does nothing
	if(result == -1)
		return;

	// print key to screen
	syscall(DRAW_CHAR_SYSCALL, key, 0x00df8090, 0x00000000, 1, 0);
}

// prints and saves to the buffer
void print_str_to_console(char * string){
	for(int i = 0; string[i] != 0; i++){
		print_char_to_console(string[i]);
	}
}

void print_number_to_console(uint64_t number){
	char buffer[12];
	int i = 0;
	if(number == 0){
		buffer[i++] = '0';
	}
	else{
		while(number > 0){
			buffer[i++] = number % 10 + '0';
			number /= 10;
		}
	}
	buffer[i] = 0;

	for(int j = i - 1; j >= 0; j--){
		print_char_to_console(buffer[j]);
	}
}

static char arguments[STRING_SIZE];
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
			running_program = 1;
			if(program->perms & DRAWING_PERMISSION){
				graphics_mode = 1;
				syscall(CLEAR_SCREEN_SYSCALL, 0x00000000, 0, 0, 0, 0);
			}
			syscall(CLEAR_KEYBOARD_BUFFER_SYSCALL, 0, 0, 0, 0, 0);
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
	case PRINT_CHAR_ENDPOINT:
		print_char_to_console((char)arg1);
		break;
	default:
		break;
	}
}

void key_handler(char event_type, int hold_times, char ascii, char scan_code){
	if(event_type != 1)
		return;

	int is_ctrl_pressed = 0;
	syscall(IS_KEY_PRESSED_SYSCALL, 0x1D, 0, &is_ctrl_pressed, 0, 0);


	// for testing purposes:
	// <1> will scroll the current line to the top, 
	// <2> will scroll up the buffer one line,
	// <3> will scroll the buffer all the way up,
	// <4> will redraw the screen
	// <5> will clear the buffer and the screen
	if(ascii == '1' && hold_times > 3){
		scroll = current_string;
		redraw();
		return;
	}
	if(ascii == '2' && hold_times > 3){
		if(scroll == oldest_string)
			return;

		if(scroll == 0)
			scroll = BUFFER_SIZE;
		scroll = scroll - 1;

		redraw();
		return;
	}
	if(ascii == '3' && hold_times > 3){
		scroll = oldest_string;
		redraw();
		return;
	}
	if(ascii == '4' && hold_times > 3){
		redraw();
		return;
	}
	if(ascii == '5' && hold_times > 3){
		clear_buffer();
		redraw();
		return;
	}

	// If ctrl + + is pressed, zoom in
	if(ascii == '+' && is_ctrl_pressed){
		syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);
		
		redraw();
		return;
	}

	// If ctrl + - is pressed, zoom out
	if(ascii == '-' && is_ctrl_pressed){
		syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);
		redraw();
		return;
	}


	// quit program when esc is pressed
	if(ascii == ASCII_ESC && running_program){
		if(hold_times > 3)
			syscall(QUIT_PROGRAM_SYSCALL, 0, 0, 0, 0, 0);
		return;
	}

	if(hold_times == 1 || KEY_REPEAT_ENABLED){
		if(!graphics_mode){
			print_char_to_console(ascii);
		}

		// si el char es imprimible, lo guardo en stdin para que lo pueda leer el programa
		if(running_program && IS_PRINTABLE(ascii)){
			add_char_to_stdin(ascii);
		}
	}

	if(ascii == '\n' && !running_program){
		execute_program(PREV_STRING);
	}
}

void status_bar_handler(RTC_Time * time){

	time_str[0] = time->hours / 10 + '0';
	time_str[1] = time->hours % 10 + '0';
	time_str[3] = time->minutes / 10 + '0';
	time_str[4] = time->minutes % 10 + '0';
	time_str[6] = time->seconds / 10 + '0';
	time_str[7] = time->seconds % 10 + '0';
	
	draw_status_bar();
}

void draw_status_bar(){
	if(graphics_mode)
		return;

	int screen_width = 0;
	syscall(GET_SCREEN_WIDTH_SYSCALL, &screen_width, 0, 0, 0, 0);

	time_position.x = getScreenWidth() - (9 * getCharWidth());

	syscall(DRAW_STRING_AT_SYSCALL, "PinkOS :)", 0x00df8090, 0x00000000, &logo_position, 0);
	syscall(DRAW_STRING_AT_SYSCALL, time_str, 0x00df8090, 0x00000000, &time_position, 0);
}


void exception_handler(int exception_id, BackupRegisters * backup_registers){

	// print the exception id
	print_str_to_console("Exception: ");
	print_char_to_console(exception_id + '0');
	print_char_to_console('\n');
	// print the backup registers
	print_str_to_console("rax: ");
	print_number_to_console(backup_registers->registers.rax);
	print_char_to_console('\n');
	print_str_to_console("rbx: ");
	print_number_to_console(backup_registers->registers.rbx);
	print_char_to_console('\n');
	print_str_to_console("rcx: ");
	print_number_to_console(backup_registers->registers.rcx);
	print_char_to_console('\n');
	print_str_to_console("rdx: ");
	print_number_to_console(backup_registers->registers.rdx);
	print_char_to_console('\n');
	print_str_to_console("rsi: ");
	print_number_to_console(backup_registers->registers.rsi);
	print_char_to_console('\n');
	print_str_to_console("rdi: ");
	print_number_to_console(backup_registers->registers.rdi);
	print_char_to_console('\n');
	print_str_to_console("rbp: ");
	print_number_to_console(backup_registers->registers.rbp);
	print_char_to_console('\n');
	print_str_to_console("r8: ");
	print_number_to_console(backup_registers->registers.r8);
	print_char_to_console('\n');
	print_str_to_console("r9: ");
	print_number_to_console(backup_registers->registers.r9);
	print_char_to_console('\n');
	print_str_to_console("r10: ");
	print_number_to_console(backup_registers->registers.r10);
	print_char_to_console('\n');
	print_str_to_console("r11: ");
	print_number_to_console(backup_registers->registers.r11);
	print_char_to_console('\n');
	print_str_to_console("r12: ");
	print_number_to_console(backup_registers->registers.r12);
	print_char_to_console('\n');
	print_str_to_console("r13: ");
	print_number_to_console(backup_registers->registers.r13);
	print_char_to_console('\n');
	print_str_to_console("r14: ");
	print_number_to_console(backup_registers->registers.r14);
	print_char_to_console('\n');
	print_str_to_console("r15: ");
	print_number_to_console(backup_registers->registers.r15);
	print_char_to_console('\n');
	print_str_to_console("cri_rip: ");
	print_number_to_console(backup_registers->cri_rip);
	print_char_to_console('\n');
	print_str_to_console("cri_rsp: ");
	print_number_to_console(backup_registers->cri_rsp);
	print_char_to_console('\n');
	print_str_to_console("cri_rflags: ");
	print_number_to_console(backup_registers->cri_rflags);
	print_char_to_console('\n');
}

void registers_handler(BackupRegisters * backup_registers){
	// print the backup registers
	save_str_to_buffer("rax: ");
	save_number_to_buffer(backup_registers->registers.rax);
	save_char_to_buffer('\n');
	save_str_to_buffer("rbx: ");
	save_number_to_buffer(backup_registers->registers.rbx);
	save_char_to_buffer('\n');
	save_str_to_buffer("rcx: ");
	save_number_to_buffer(backup_registers->registers.rcx);
	save_char_to_buffer('\n');
	save_str_to_buffer("rdx: ");
	save_number_to_buffer(backup_registers->registers.rdx);
	save_char_to_buffer('\n');
	save_str_to_buffer("rsi: ");
	save_number_to_buffer(backup_registers->registers.rsi);
	save_char_to_buffer('\n');
	save_str_to_buffer("rdi: ");
	save_number_to_buffer(backup_registers->registers.rdi);
	save_char_to_buffer('\n');
	save_str_to_buffer("rbp: ");
	save_number_to_buffer(backup_registers->registers.rbp);
	save_char_to_buffer('\n');
	save_str_to_buffer("r8: ");
	save_number_to_buffer(backup_registers->registers.r8);
	save_char_to_buffer('\n');
	save_str_to_buffer("r9: ");
	save_number_to_buffer(backup_registers->registers.r9);
	save_char_to_buffer('\n');
	save_str_to_buffer("r10: ");
	save_number_to_buffer(backup_registers->registers.r10);
	save_char_to_buffer('\n');
	save_str_to_buffer("r11: ");
	save_number_to_buffer(backup_registers->registers.r11);
	save_char_to_buffer('\n');
	save_str_to_buffer("r12: ");
	save_number_to_buffer(backup_registers->registers.r12);
	save_char_to_buffer('\n');
	save_str_to_buffer("r13: ");
	save_number_to_buffer(backup_registers->registers.r13);
	save_char_to_buffer('\n');
	save_str_to_buffer("r14: ");
	save_number_to_buffer(backup_registers->registers.r14);
	save_char_to_buffer('\n');
	save_str_to_buffer("r15: ");
	save_number_to_buffer(backup_registers->registers.r15);
	save_char_to_buffer('\n');
	save_str_to_buffer("rip: ");
	save_number_to_buffer(backup_registers->cri_rip);
	save_char_to_buffer('\n');
	save_str_to_buffer("rsp: ");
	save_number_to_buffer(backup_registers->cri_rsp);
	save_char_to_buffer('\n');
	if(!graphics_mode){
		redraw();
	}
}

// configures the current line as a prompt, and prints a graphical indicator of that
void newPrompt(){
	// print the prompt
	is_input[current_string] = 1;
	syscall(DRAW_STRING_SYSCALL, default_prompt, 0x00df8090, 0x00000000, 0, 0);
}


int i = 0;
void restoreContext(uint8_t was_graphic){
	if(was_graphic){
		graphics_mode = 0;
		redraw();
	}
	print_char_to_console('\n');
	newPrompt();
	running_program = 0;
	clear_stdin();
	
	idle("idle from restoreContext");
}

// message for debugging purposes
void idle(char * message){
	while(1){
		// if (message != 0){
		// 	syscall(DRAW_STRING_SYSCALL, message, 0x00df8090, 0x00000000, 0, 0);
		// }
		_hlt();
	}
}

int main() {
	// Set userland stack base, to allways start programs here and to return here from exceptions or program termination
	syscall(SET_SYSTEM_STACK_BASE_SYSCALL, get_stack_pointer(),0,0,0,0);

	syscall(SET_CURSOR_LINE_SYSCALL, 1, 0, 0, 0, 0); // evita dibujar la status bar
	newPrompt();
	
	// Setea todos los handlers, para quedar corriendo "en el fondo" <- bueno cambio de idioma cuando se me canta el ogt
	syscall(SET_HANDLER_SYSCALL, 0, key_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 2, status_bar_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 3, restoreContext, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 4, api_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 5, exception_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 6, registers_handler, 0, 0, 0);

	idle("idle from main");
}
