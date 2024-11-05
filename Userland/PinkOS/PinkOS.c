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
#include <audioLib.h>
#include <pictures.h>

#define PREV_STRING current_string > 0 ? current_string - 1 : BUFFER_SIZE - 1
#define ADVANCE_INDEX(index) index = (index + 1) % BUFFER_SIZE;
#define DECREASE_INDEX(index) index = index ? (index - 1) : BUFFER_SIZE - 1;

#define IS_ASCII(ascii) ((unsigned char)ascii > 0 && (unsigned char)ascii < 256)
#define IS_PRINTABLE_CHAR(ascii) ((unsigned char)ascii >= 32 && (unsigned char)ascii < 255 && (unsigned char)ascii != ASCII_DEL)

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
extern void *get_stack_pointer();
extern void _hlt();

static int background_audio_enabled = 0;
static struct PreviousAudioState{
	uint8_t restoring_audio;
	uint8_t playing;
	AudioState state;
} previousAudioState = {0};

static int show_home_screen = 1;

typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t day_of_week;
} RTC_Time;

typedef struct
{
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
} Registers;

typedef struct
{
	Registers registers;
	uint64_t cri_rip, cri_rsp, cri_rflags;
} BackupRegisters;

int running_program = 0; // 0 if no program is running (besides the shell itself, ofc)
int graphics_mode = 0;	 // 0 for CLI, 1 for GUI

#define BUFFER_SIZE 500
#define STRING_SIZE 200		 // 199 usable characters and the null termination
#define KEY_REPEAT_ENABLED 1 // 0 for disabling key repeat

static char time_str[10] = "  00:00:00";
static Point time_position = {950, 0};
static Point logo_position = {10, 0};


static const unsigned char *command_not_found_msg = "Command not found\n";
static const unsigned char *default_prompt = " > ";

unsigned char buffer[BUFFER_SIZE][STRING_SIZE] = {0};
uint8_t is_input[BUFFER_SIZE] = {0}; // 1 if the string is an input, 0 if it's an output
int current_string = 0;
int current_position = 0;
int oldest_string = 0;

unsigned char stdin_buffer[STRING_SIZE] = {0};
int stdin_write_position = 0;
int stdin_read_position = 0;

unsigned char get_char_from_stdin()
{
	if (stdin_write_position == stdin_read_position)
	{
		return 0;
	}

	unsigned char c = stdin_buffer[stdin_read_position];
	stdin_read_position = (stdin_read_position + 1) % STRING_SIZE;
	return c;
}

void add_char_to_stdin(unsigned char c)
{
	stdin_buffer[stdin_write_position] = c;
	stdin_write_position = (stdin_write_position + 1) % STRING_SIZE;
	if (stdin_write_position == stdin_read_position)
	{
		stdin_read_position = (stdin_read_position + 1) % STRING_SIZE;
	}
}

void clear_stdin()
{
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
int save_char_to_buffer(unsigned char key)
{
	// if enter key is pressed, move to the next string
	if (key == '\n')
	{
		ADVANCE_INDEX(current_string) // move to the next string

		// if the buffer is full, the oldest string gets overwritten, move the oldest string index to the next string
		// and automatically scroll if a visible line is overwritten
		if (current_string == oldest_string)
		{
			ADVANCE_INDEX(oldest_string)

			// TODO: ver si este código está bien acá o es más prolijo abstraer la lógica del scroll
			// igual la única forma de que esto pase es que el buffer sea más chico que la pantalla
			if (oldest_string == scroll)
			{
				ADVANCE_INDEX(scroll)
			}
		}

		// reset the current position and terminate the string with 0
		current_position = 0;
		buffer[current_string][current_position] = 0;
		is_input[current_string] = 0; // lines are not input until assigned as such by newPrompt()
	}
	// backspace
	else if (key == 8)
	{
		if (current_position > 0)
		{
			current_position--;
			buffer[current_string][current_position] = 0;
		}
	}
	else if (IS_PRINTABLE_CHAR(key))
	{
		// checks if the buffer is full, if it is, key presses are ignored
		if (current_position == STRING_SIZE - 1)
			return -1;

		// save key to buffer
		buffer[current_string][current_position] = key;
		current_position++;
		buffer[current_string][current_position] = 0;
	}
	else
	{
		// unsupported key
		return -1;
	}

	return 0;
}

int save_str_to_buffer(unsigned char *string)
{
	for (int i = 0; string[i] != 0; i++)
	{
		save_char_to_buffer(string[i]);
	}
}

int save_number_to_buffer(uint64_t number)
{
	unsigned char buffer[12];
	int i = 0;
	if (number == 0)
	{
		buffer[i++] = '0';
	}
	else
	{
		while (number > 0)
		{
			buffer[i++] = number % 10 + '0';
			number /= 10;
		}
	}
	buffer[i] = 0;

	for (int j = i - 1; j >= 0; j--)
	{
		save_char_to_buffer(buffer[j]);
	}
}

void redraw()
{
	// clear screen
	syscall(CLEAR_SCREEN_SYSCALL, 0x00000000, 0, 0, 0, 0);
	draw_status_bar();
	syscall(SET_CURSOR_LINE_SYSCALL, 1, 0, 0, 0, 0); // evita dibujar la status bar

	// print the buffer from the scroll position to the current string

	int i = scroll ? scroll - 1 : BUFFER_SIZE - 1;
	do
	{
		ADVANCE_INDEX(i)

		if (is_input[i] == 1)
		{
			syscall(DRAW_STRING_SYSCALL, default_prompt, 0x00df8090, 0x00000000, 0, 0);
		}
		syscall(DRAW_STRING_SYSCALL, buffer[i], 0x00df8090, 0x00000000, 0, 0);

		// print a new line (except in the last string)
		if (i != current_string)
			syscall(DRAW_CHAR_SYSCALL, '\n', 0x00df8090, 0x00000000, 1, 0);

	} while (i != current_string);
}

void clear_buffer()
{
	// clear the buffer
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		buffer[i][0] = 0;
	}
	current_position = 0;
	current_string = 0;
	oldest_string = 0;
	scroll = 0;
}

void clear_console()
{
	save_char_to_buffer('\n');
	scroll = current_string;
	// newPrompt();
	redraw();
}

// prints and saves to the buffer
void add_char_to_stdout(unsigned char *character)
{
	if(!IS_ASCII(character)) return;

	// presionar delete en la primera posición no hace nada
	if (character == '\b' && current_position == 0)
	{
		return;
	}

	// si el caracter haría wrappeo, scrolleo una línea

	int result = save_char_to_buffer(character);

	// if the string limit is reached or the character is unsupported, it simply does nothing
	if (result == -1)
		return;

	// print character to screen, if not in graphic mode
	if(!graphics_mode) syscall(DRAW_CHAR_SYSCALL, character, 0x00df8090, 0x00000000, 1, 0);
}

// prints and saves to the buffer
void add_str_to_stdout(unsigned char *string)
{
	for (int i = 0; string[i] != 0; i++)
	{
		add_char_to_stdout(string[i]);
	}
}

void add_number_to_stdout(uint64_t number)
{
	unsigned char buffer[12];
	int i = 0;
	if (number == 0)
	{
		buffer[i++] = '0';
	}
	else
	{
		while (number > 0)
		{
			buffer[i++] = number % 10 + '0';
			number /= 10;
		}
	}
	buffer[i] = 0;

	for (int j = i - 1; j >= 0; j--)
	{
		add_char_to_stdout(buffer[j]);
	}
}

static unsigned char arguments[STRING_SIZE];
void execute_program(int input_line)
{
	// get the program name
	unsigned char program_name[STRING_SIZE];
	int i = 0;
	for (; buffer[input_line][i] != ' ' && buffer[input_line][i] != 0; i++)
	{
		program_name[i] = buffer[input_line][i];
	}
	program_name[i] = 0;

	if (buffer[input_line][i] == ' ')
		i++;

	// get the arguments

	int j = 0;
	for (; buffer[input_line][i] != 0; i++, j++)
	{
		arguments[j] = buffer[input_line][i];
	}
	arguments[j] = 0;

	// get the program entry point
	Program *program = get_program_entry(program_name);

	// if the program is not found, print an error message
	if (program == 0)
	{
		// "Command not found"
		save_str_to_buffer(command_not_found_msg);
		syscall(DRAW_STRING_SYSCALL, command_not_found_msg, 0x00df8090, 0x00000000, 0, 0);
		newPrompt();
	}
	// if the program is found, execute it
	else
	{
		running_program = 1;

		if (program->perms & DRAWING_PERMISSION)
		{
			graphics_mode = 1;
			syscall(CLEAR_SCREEN_SYSCALL, 0x00000000, 0, 0, 0, 0);
		}
		if ((program->perms & PLAY_AUDIO_PERMISSION) && background_audio_enabled)
		{
			// Si voy a ejecutar un programa con permisos de audio, y se estaba reproduciendo algo en segundo plano, guardo el estado
			// E indico que luego de terminar la ejecución debe restaurarse el estado guardado
			disableBackgroundAudio();
			previousAudioState.restoring_audio = 1;
			previousAudioState.playing = is_audio_playing();
			pause_audio();
			previousAudioState.state = get_audio_state();
		}
		syscall(CLEAR_KEYBOARD_BUFFER_SYSCALL, 0, 0, 0, 0, 0);
		syscall(RUN_PROGRAM_SYSCALL, program, arguments, 0, 0, 0);
	}
}


void api_handler(uint64_t endpoint_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
	switch (endpoint_id)
	{
	case CLEAR_SCREEN_ENDPOINT:
		clear_console();
		break;
	case PRINT_STRING_ENDPOINT:
		add_str_to_stdout(arg1);
		break;
	case PRINT_CHAR_ENDPOINT:
		add_char_to_stdout(arg1);
		break;
	case ENABLE_BACKGROUND_AUDIO_ENDPOINT:
		background_audio_enabled = 1;
		break;
	case DISABLE_BACKGROUND_AUDIO_ENDPOINT:
		background_audio_enabled = 0;
		break;
	default:
		break;
	}
}

void key_handler(unsigned char event_type, int hold_times, unsigned char ascii, unsigned char scan_code)
{
	if (event_type != 1 && event_type != 3)  // just register press events (not release or null events)
		return;

	// syscall(DRAW_HEX_SYSCALL, scan_code, 0x00df8090, 0x00000000, 0, 0);  // For debugging
	// return;


	// --- HOLDING ESC FORCE QUITS THE CURRENT PROGRAM ---
	if (ascii == ASCII_ESC && running_program && hold_times > 1)
	{
		syscall(QUIT_PROGRAM_SYSCALL, 0, 0, 0, 0, 0);
		return;
	}
	
	int is_ctrl_pressed, is_shift_pressed = 0;
	syscall(IS_KEY_PRESSED_SYSCALL, 0x1D, 0, &is_ctrl_pressed, 0, 0);
	syscall(IS_KEY_PRESSED_SYSCALL, 0x2A, 0, &is_shift_pressed, 0, 0);

	// --- HANDLE SHELL KEYBOARD SHORTCUTS ---
	if(is_ctrl_pressed && !graphics_mode)
	{
		// --- FONT SIZE ---
		if (ascii == '+' && hold_times == 1)
			syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);  // If ctrl + '+' is pressed, zoom in
		else if (ascii == '-' && hold_times == 1)
			syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);  // If ctrl + '-' is pressed, zoom out

		// --- SCROLL ---
		else if((scan_code == 0x48 && event_type == 3))
		{
			if(scroll == oldest_string) return;  			// don't scroll up if the oldest line is at the top
			if(is_shift_pressed) scroll = oldest_string;	// with shift + ctrl + up, scroll to the top
			else DECREASE_INDEX(scroll);
		}
		else if(scan_code == 0x50 && event_type == 3)
		{
			if(scroll == current_string) return;  			// don't scroll down if the current line is at the bottom
			if(is_shift_pressed) scroll = current_string;   // whith shift + ctrl + down, scroll to the bottom
			else ADVANCE_INDEX(scroll);
		}
		else if(ascii == 'l')
		{
			scroll = current_string;
		}else{
			return;
		}

		redraw();
		return;
	}

	// --- MANEJA LA ENTRADA ESTÁNDAR Y EL BUFFER DE LA TERMINAL ---
	// 		El key repeat es configurable, 
	// 		O sea que podés decidir si mantener una tecla presionada solo mande la interrupción la primera vez
	if (hold_times == 1 || KEY_REPEAT_ENABLED || ascii == ASCII_BS)
	{
		// Solo guardo en el buffer de la terminal si estoy en modo CLI
		if (!graphics_mode) add_char_to_stdout(ascii);

		// Si hay un programa corriendo y la entrada es ascii, se lo mando al programa por stdin
		if (running_program && ascii)  add_char_to_stdin(ascii);
	}
	
	// --- ENTER TO EXECUTE ---
	if (ascii == '\n' && !running_program) execute_program(PREV_STRING);
}

void status_bar_handler(RTC_Time *time)
{
	time_str[0] = is_audio_playing() ? 128 : ' ';

	time_str[2] = time->hours / 10 + '0';
	time_str[3] = time->hours % 10 + '0';
	time_str[5] = time->minutes / 10 + '0';
	time_str[6] = time->minutes % 10 + '0';
	time_str[8] = time->seconds / 10 + '0';
	time_str[9] = time->seconds % 10 + '0';

	draw_status_bar();
}

void draw_status_bar()
{
	if (graphics_mode)
		return;

	int screen_width = 0;
	syscall(GET_SCREEN_WIDTH_SYSCALL, &screen_width, 0, 0, 0, 0);

	time_position.x = getScreenWidth() - (11 * getCharWidth());
	syscall(DRAW_STRING_AT_SYSCALL, "PinkOS :)", 0x00df8090, 0x00000000, &logo_position, 0);
	syscall(DRAW_STRING_AT_SYSCALL, time_str, 0x00df8090, 0x00000000, &time_position, 0);
}

void exception_handler(int exception_id, BackupRegisters *backup_registers)
{
	stop_audio();
	disableBackgroundAudio();

	// TODO: Implementar Pantallazo Rosa
	// print the exception id
	add_str_to_stdout("Exception: ");
	add_char_to_stdout(exception_id + '0');
	add_char_to_stdout('\n');
	// print the backup registers
	add_str_to_stdout("rax: ");
	add_number_to_stdout(backup_registers->registers.rax);
	add_char_to_stdout('\n');
	add_str_to_stdout("rbx: ");
	add_number_to_stdout(backup_registers->registers.rbx);
	add_char_to_stdout('\n');
	add_str_to_stdout("rcx: ");
	add_number_to_stdout(backup_registers->registers.rcx);
	add_char_to_stdout('\n');
	add_str_to_stdout("rdx: ");
	add_number_to_stdout(backup_registers->registers.rdx);
	add_char_to_stdout('\n');
	add_str_to_stdout("rsi: ");
	add_number_to_stdout(backup_registers->registers.rsi);
	add_char_to_stdout('\n');
	add_str_to_stdout("rdi: ");
	add_number_to_stdout(backup_registers->registers.rdi);
	add_char_to_stdout('\n');
	add_str_to_stdout("rbp: ");
	add_number_to_stdout(backup_registers->registers.rbp);
	add_char_to_stdout('\n');
	add_str_to_stdout("r8: ");
	add_number_to_stdout(backup_registers->registers.r8);
	add_char_to_stdout('\n');
	add_str_to_stdout("r9: ");
	add_number_to_stdout(backup_registers->registers.r9);
	add_char_to_stdout('\n');
	add_str_to_stdout("r10: ");
	add_number_to_stdout(backup_registers->registers.r10);
	add_char_to_stdout('\n');
	add_str_to_stdout("r11: ");
	add_number_to_stdout(backup_registers->registers.r11);
	add_char_to_stdout('\n');
	add_str_to_stdout("r12: ");
	add_number_to_stdout(backup_registers->registers.r12);
	add_char_to_stdout('\n');
	add_str_to_stdout("r13: ");
	add_number_to_stdout(backup_registers->registers.r13);
	add_char_to_stdout('\n');
	add_str_to_stdout("r14: ");
	add_number_to_stdout(backup_registers->registers.r14);
	add_char_to_stdout('\n');
	add_str_to_stdout("r15: ");
	add_number_to_stdout(backup_registers->registers.r15);
	add_char_to_stdout('\n');
	add_str_to_stdout("cri_rip: ");
	add_number_to_stdout(backup_registers->cri_rip);
	add_char_to_stdout('\n');
	add_str_to_stdout("cri_rsp: ");
	add_number_to_stdout(backup_registers->cri_rsp);
	add_char_to_stdout('\n');
	add_str_to_stdout("cri_rflags: ");
	add_number_to_stdout(backup_registers->cri_rflags);
	add_char_to_stdout('\n');
}

void registers_handler(BackupRegisters *backup_registers)
{
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

	if (!graphics_mode)
	{
		redraw();
		newPrompt();
	}
}

// configures the current line as a prompt, and prints a graphical indicator of that
void newPrompt()
{
	// print the prompt
	is_input[current_string] = 1;
	syscall(DRAW_STRING_SYSCALL, default_prompt, 0x00df8090, 0x00000000, 0, 0);
}

int i = 0;
void restoreContext(uint8_t was_graphic)
{
	if (was_graphic)
	{
		graphics_mode = 0;
		redraw();
	}
	add_char_to_stdout('\n');
	newPrompt();
	running_program = 0;
	clear_stdin();

	// Si el programa no activó el audio en segundo plano, pauso el sonido que se haya dejado reproduciendo
	// Si en el estado anterior a la ejecución del programa se estaba reproduciendo audio en segundo plano, continuar la reproducción
	if (!background_audio_enabled){
		stop_audio();
		if(previousAudioState.restoring_audio){
			enableBackgroundAudio();
			load_audio_state(previousAudioState.state);
			if(previousAudioState.playing) resume_audio();
		}
	}
	previousAudioState.restoring_audio = 0;

	idle("idle from restoreContext");
}

// message for debugging purposes
void idle(unsigned char *message)
{
	while (1)
	{
		// if (message != 0){
		// 	syscall(DRAW_STRING_SYSCALL, message, 0x00df8090, 0x00000000, 0, 0);
		// }
		_hlt();
	}
}

void home_screen_exit_handler(unsigned char event_type, int hold_times, unsigned char ascii, unsigned char scan_code)
{
	show_home_screen = 0;
}

void home_screen()
{

	syscall(SET_HANDLER_SYSCALL, 0, home_screen_exit_handler, 0, 0, 0);

	Point position = {0};
	int scale = 12;

	int screen_width = getScreenWidth();
	int screen_height = getScreenHeight();

	position.x = (screen_width - MONA_LISA_WIDTH * scale) / 2;
	position.y = (screen_height - MONA_LISA_HEIGHT * scale) / 2;

	drawBitmap(mona_lisa, MONA_LISA_WIDTH, MONA_LISA_HEIGHT, position, scale);

	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);
	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);

	// center the text
	position.x = 50;
	position.y = 400;

	// draw a big welcome message
	syscall(DRAW_STRING_AT_SYSCALL, "Welcome to PinkOS!", 0x00df8090, 0x00000000, &position, 0);

	position.y += 50;

	syscall(DRAW_STRING_AT_SYSCALL, "Press any key to continue", 0x00df8090, 0x00000000, &position, 0);

	syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);
	syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);

	while (show_home_screen)
	{
		_hlt();
	}
}

int main()
{
	// Set userland stack base, to allways start programs here and to return here from exceptions or program termination
	syscall(SET_SYSTEM_STACK_BASE_SYSCALL, get_stack_pointer(), 0, 0, 0, 0);
	syscall(SET_CURSOR_LINE_SYSCALL, 1, 0, 0, 0, 0); // evita dibujar la status bar (sí, cambio de idioma cuando se me canta el ogt)

	home_screen();
	redraw();

	// Setea todos los handlers, para quedar corriendo "en el fondo"
	syscall(SET_HANDLER_SYSCALL, 5, exception_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 6, registers_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 4, api_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 0, key_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 2, status_bar_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, 3, restoreContext, 0, 0, 0);

	add_str_to_stdout("\n * This system has a * 90% humor setting * ...\n * but only 100% style.\n");
	add_str_to_stdout("\n * Type help for help\n");
	newPrompt();

	idle("idle from main");
}
