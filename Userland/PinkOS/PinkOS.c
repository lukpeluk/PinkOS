/* PinkOS.c */
#include <stdint.h>
#include <handlerIds.h>
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
#include <colors.h>

#define PREV_STRING current_string > 0 ? current_string - 1 : BUFFER_SIZE - 1
#define ADVANCE_INDEX(index, array_size) index = (index + 1) % array_size;
#define DECREASE_INDEX(index, array_size) index = index ? (index - 1) : array_size - 1;

#define IS_ASCII(ascii) ((char)ascii > 0 && (char)ascii < 256)
#define IS_PRINTABLE_CHAR(ascii) ((char)ascii >= 32 && (char)ascii < 255 && (char)ascii != ASCII_DEL)

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
extern void *get_stack_pointer();
extern void _hlt();

static int background_audio_enabled = 0;
static struct PreviousAudioState{
	uint8_t restoring_audio;
	uint8_t playing;
	AudioState state;
} previousAudioState = {0};

Colors* ColorSchema = &PinkOSMockupColors;

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
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
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

#define COMMAND_BUFFER_SIZE 10

static char logo_str[10] = "    PinkOS";
static char time_str[10] = "  00:00:00";
static Point time_position = {950, 5};
static Point logo_position = {10, 5};


static const char *command_not_found_msg = (const char *) ">?Command not found\n";
static const char *default_prompt = (const char *)" > ";

char buffer[BUFFER_SIZE][STRING_SIZE] = {0};
uint8_t is_input[BUFFER_SIZE] = {0}; // 1 if the string is an input, 0 if it's an output
int current_string = 0;
int current_position = 0;
int oldest_string = 0;
int scroll = 0; // indica qué línea es la que está en la parte superior de la pantalla

uint32_t current_text_color; // intended to be changed via markup
int highlighting_text = 0;	 // for highlighting text

char command_buffer[COMMAND_BUFFER_SIZE][STRING_SIZE] = {0};
int current_command = 0;
int oldest_command = 0;
int command_in_iteration = -1;

char stdin_buffer[STRING_SIZE] = {0};
int stdin_write_position = 0;
int stdin_read_position = 0;

void draw_status_bar();
void newPrompt();
void idle(char *message);

char get_char_from_stdin()
{
	if (stdin_write_position == stdin_read_position)
	{
		return 0;
	}

	char c = stdin_buffer[stdin_read_position];
	stdin_read_position = (stdin_read_position + 1) % STRING_SIZE;
	return c;
}

void add_char_to_stdin(char c)
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
int save_char_to_buffer(char key)
{
	// if enter key is pressed, move to the next string
	if (key == '\n')
	{
		ADVANCE_INDEX(current_string, BUFFER_SIZE) // move to the next string

		// if the buffer is full, the oldest string gets overwritten, move the oldest string index to the next string
		// and automatically scroll if a visible line is overwritten
		if (current_string == oldest_string)
		{
			ADVANCE_INDEX(oldest_string, BUFFER_SIZE)

			// TODO: ver si este código está bien acá o es más prolijo abstraer la lógica del scroll
			// igual la única forma de que esto pase es que el buffer sea más chico que la pantalla
			if (oldest_string == scroll)
			{
				ADVANCE_INDEX(scroll, BUFFER_SIZE)
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

int save_str_to_buffer(char *string)
{
	for (int i = 0; string[i] != 0; i++)
	{
		save_char_to_buffer(string[i]);
	}
	return 0;
}

int save_number_to_buffer(uint64_t number)
{
	char buffer[12];
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
	return 0;
}

void reset_markup(){
	current_text_color = ColorSchema->text;
	highlighting_text = 0;
}

// recieves a string and updates the current color and highlighting state if the string contains markup at the beginning
// returns the amount of markup chars detected, to avoid printing them
int process_markup(char *string)
{
	int markup_chars = 2; // 2 is the default amount of markup characters

	if(*string == '=' && *(string + 1) == '=')
		highlighting_text = !highlighting_text;

	else if(*string == '>' && *(string + 1) == '!')
		current_text_color = ColorSchema->error;

	else if(*string == '>' && *(string + 1) == '?')
		current_text_color = ColorSchema->warning;

	else if(*string == '>' && *(string + 1) == '+')
		current_text_color = ColorSchema->success;

	else if(*string == '>' && *(string + 1) == '.')
		current_text_color = ColorSchema->text;
	else if(*string == '>' && *(string + 1) == '#')
		current_text_color = ColorSchema->info;
	
	else markup_chars = 0;

	return markup_chars;
}

void redraw()
{
	// clear screen
	syscall(CLEAR_SCREEN_SYSCALL, (uint64_t)ColorSchema->background, 0, 0, 0, 0);
	draw_status_bar();
	syscall(SET_CURSOR_LINE_SYSCALL, 2, 0, 0, 0, 0); // evita dibujar la status bar
	reset_markup(); // just in case

	// print the buffer from the scroll position to the current string

	int i = scroll ? scroll - 1 : BUFFER_SIZE - 1;
	do
	{
		ADVANCE_INDEX(i, BUFFER_SIZE)

		if (is_input[i] == 1)
		{
			syscall(DRAW_STRING_SYSCALL, (uint64_t)default_prompt, (uint64_t)ColorSchema->prompt, (uint64_t)ColorSchema->background, 0, 0);
		}
		int j = 0;
		int markup_chars = 0;
		for (; buffer[i][j] != 0; j++)
		{
			if(!(i == current_string))
				markup_chars = process_markup(buffer[i] + j) ; // updates current color and highlighting state if the string contains markup in the current position

			if(markup_chars)
				j += markup_chars-1; // updates the index to avoid printing the markup characters
			else
				syscall(DRAW_CHAR_SYSCALL, (uint64_t)buffer[i][j], (uint64_t)current_text_color, (uint64_t)(highlighting_text ? ColorSchema->highlighted_text_background : ColorSchema->background), 1, 0);
		}
		reset_markup(); // resets the styles in case it was not reset manually

		// print a new line (except in the last string)
		if (i != current_string)
			syscall(DRAW_CHAR_SYSCALL, (uint64_t)'\n', (uint64_t)current_text_color, (uint64_t)ColorSchema->background, 1, 0);

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
	reset_markup();
}

void clear_console()
{
	scroll = current_string;
	reset_markup();
	redraw();
}

void scroll_if_out_of_bounds()
{	
	// --- AUTO SCROLL ---

	int is_in_boundaries = -1;
	uint8_t needs_redraw = 0;
	do {
		uint64_t cursor_line = 0, cursor_col = 0;
		syscall(GET_CURSOR_LINE_SYSCALL, (uint64_t)&cursor_line, 0, 0, 0, 0);
		syscall(GET_CURSOR_COL_SYSCALL, (uint64_t)&cursor_col, 0, 0, 0, 0);
		
		syscall(IS_CURSOR_IN_BOUNDARIES_SYSCALL, cursor_line, cursor_col + 1, (uint64_t)&is_in_boundaries, 0, 0);

		if(!is_in_boundaries){
			ADVANCE_INDEX(scroll, BUFFER_SIZE);
			needs_redraw = 1;
			syscall(SET_CURSOR_LINE_SYSCALL, cursor_line - 1, 0, 0, 0, 0);
		}
	} while(!is_in_boundaries);
	
	if(needs_redraw){
		redraw();
	}
}

// prints and saves to the buffer
void add_char_to_stdout(char character)
{
	if(!IS_ASCII(character)) return;

	// presionar delete en la primera posición no hace nada
	if (character == '\b' && current_position == 0)
		return;

	// a new line allways resets the markup
	if (character == '\n')
		reset_markup();

	scroll_if_out_of_bounds();

	int result = save_char_to_buffer(character);

	// if the string limit is reached or the character is unsupported, it simply does nothing
	if (result == -1)
		return;

	// print character to screen, if not in graphic mode
	if(!graphics_mode)
		syscall(DRAW_CHAR_SYSCALL, (uint64_t)character, (uint64_t)current_text_color, (uint64_t)(highlighting_text ? ColorSchema->highlighted_text_background : ColorSchema->background), 1, 0);
}

// prints and saves to the buffer
void add_str_to_stdout(char *string)
{
	int markup_chars = 0;
	for (int i = 0; string[i] != 0; i++)
	{
		markup_chars = process_markup(string + i); // updates current color and highlighting state if the string contains markup in the current position
		
		if(markup_chars) 
			i += markup_chars-1; // updates the index to avoid printing the markup characters
		else
			add_char_to_stdout(string[i]);

		// markups are not printed but need to be saved to the buffer
		while(markup_chars--){
			save_char_to_buffer(string[i-markup_chars]);
			scroll_if_out_of_bounds();
		}
	}
	// reset_markup(); // no estoy seguro si queremos que cada impresión resetee el estilo
}

void add_number_to_stdout(uint64_t number)
{
	char buffer[12];
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

static char arguments[STRING_SIZE];
void execute_program(int input_line)
{
	// get the program name
	char program_name[STRING_SIZE];
	int i = 0;
	for (; buffer[input_line][i] != ' ' && buffer[input_line][i] != 0; i++)
	{
		program_name[i] = buffer[input_line][i];
	}
	program_name[i] = 0;

	if (buffer[input_line][i] == ' '){
		i++;
	}

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
		add_str_to_stdout((char *)command_not_found_msg);
		newPrompt();
	}
	// if the program is found, execute it
	else
	{
		running_program = 1;

		if (program->perms & DRAWING_PERMISSION)
		{
			graphics_mode = 1;
			syscall(CLEAR_SCREEN_SYSCALL, (uint64_t)ColorSchema->background, 0, 0, 0, 0);
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
		syscall(RUN_PROGRAM_SYSCALL, (uint64_t)program, (uint64_t)arguments, 0, 0, 0);
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
		add_str_to_stdout((char *)arg1);
		break;
	case PRINT_CHAR_ENDPOINT:
		add_char_to_stdout((char)arg1);
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

void key_handler(char event_type, int hold_times, char ascii, char scan_code)
{
	if (event_type != 1 && event_type != 3)  // just register press events (not release or null events)
		return;

	// syscall(DRAW_HEX_SYSCALL, scan_code, ColorSchema->text, ColorSchema->background, 0, 0);  // For debugging
	// return;


	// --- HOLDING ESC FORCE QUITS THE CURRENT PROGRAM ---
	if (ascii == ASCII_ESC && running_program && hold_times > 1)
	{
		syscall(QUIT_PROGRAM_SYSCALL, 0, 0, 0, 0, 0);
		return;
	}
	
	int is_ctrl_pressed, is_shift_pressed = 0;
	syscall(IS_KEY_PRESSED_SYSCALL, 0x1D, 0, (uint64_t)&is_ctrl_pressed, 0, 0);
	syscall(IS_KEY_PRESSED_SYSCALL, 0x2A, 0, (uint64_t)&is_shift_pressed, 0, 0);


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
			else DECREASE_INDEX(scroll, BUFFER_SIZE);
		}
		else if(scan_code == 0x50 && event_type == 3)
		{
			if(scroll == current_string) return;  			// don't scroll down if the current line is at the bottom
			if(is_shift_pressed) scroll = current_string;   // whith shift + ctrl + down, scroll to the bottom
			else ADVANCE_INDEX(scroll, BUFFER_SIZE);
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

	// --- HANDLE ARROWS FOR PREVIOUS COMMANDS ---
	if(scan_code == 0x48){
		// WIP - ITERAR POR LOS COMANDOS RECIENTES
		// print("UP\n");
		// if(command_in_iteration != oldest_command){
		// 	command_in_iteration = current_command ? current_command - 1 : COMMAND_BUFFER_SIZE - 1;
		// 	buffer[current_string][0] = 0;
		// 	add_str_to_stdout(command_buffer[command_in_iteration]);
		// 	DECREASE_INDEX(command_in_iteration, COMMAND_BUFFER_SIZE);
		// }
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
	if (ascii == '\n' && !running_program) {
		// WIP - ITERAR POR LOS COMANDOS RECIENTES
		// Al tocar enter se guarda la línea en el buffer de comandos
		// strcpy(command_buffer[current_command], PREV_STRING);
		// ADVANCE_INDEX(current_command, COMMAND_BUFFER_SIZE);
		// if(current_command == oldest_command){
		// 	ADVANCE_INDEX(oldest_command, COMMAND_BUFFER_SIZE);
		// }

		// // Si se está iterando por los comandos anteriores, se ejecuta el comando actual
		// if(command_in_iteration != -1){
		// 	command_in_iteration = -1;
		// 	execute_program(command_buffer[command_in_iteration]);
		// }

		redraw(); // redibuja para que se parsee el marcado, TODO: tratar que solo redibuje la linea actual en vez de toda la pantalla
		execute_program(PREV_STRING);
	}
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
	syscall(GET_SCREEN_WIDTH_SYSCALL, (uint64_t)&screen_width, 0, 0, 0, 0);

	int char_width = getCharWidth();
	time_position.x = getScreenWidth() - (11 * char_width);
	logo_str[2] = 169;

	drawRectangle(ColorSchema->status_bar_background, screen_width, char_width + 10, (Point){0, 0});
	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)logo_str, (uint64_t)ColorSchema->status_bar_text, (uint64_t)ColorSchema->status_bar_background, (uint64_t)&logo_position, 0);
	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)time_str, (uint64_t)ColorSchema->status_bar_text, (uint64_t)ColorSchema->status_bar_background, (uint64_t)&time_position, 0);
}

void exception_handler(int exception_id, BackupRegisters *backup_registers)
{
	stop_audio();
	background_audio_enabled = 0;
	running_program = 0;
	graphics_mode = 0;

	// TODO: capaz hacer función add_warning_to_stdout o algo así para no poner >! en todos lados

	// TODO: Implementar Pantallazo Rosa
	add_str_to_stdout((char *)">!Exception: ");
	add_number_to_stdout(exception_id);
	add_char_to_stdout('\n');

	// print the backup registers
	add_str_to_stdout((char *)">!rax: ");
	add_number_to_stdout(backup_registers->registers.rax);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!rbx: ");
	add_number_to_stdout(backup_registers->registers.rbx);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!rcx: ");
	add_number_to_stdout(backup_registers->registers.rcx);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!rdx: ");
	add_number_to_stdout(backup_registers->registers.rdx);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!rsi: ");
	add_number_to_stdout(backup_registers->registers.rsi);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!rdi: ");
	add_number_to_stdout(backup_registers->registers.rdi);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!rbp: ");
	add_number_to_stdout(backup_registers->registers.rbp);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r8: ");
	add_number_to_stdout(backup_registers->registers.r8);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r9: ");
	add_number_to_stdout(backup_registers->registers.r9);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r10: ");
	add_number_to_stdout(backup_registers->registers.r10);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r11: ");
	add_number_to_stdout(backup_registers->registers.r11);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r12: ");
	add_number_to_stdout(backup_registers->registers.r12);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r13: ");
	add_number_to_stdout(backup_registers->registers.r13);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r14: ");
	add_number_to_stdout(backup_registers->registers.r14);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!r15: ");
	add_number_to_stdout(backup_registers->registers.r15);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!cri_rip: ");
	add_number_to_stdout(backup_registers->cri_rip);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!cri_rsp: ");
	add_number_to_stdout(backup_registers->cri_rsp);
	add_char_to_stdout('\n');
	add_str_to_stdout((char *)">!cri_rflags: ");
	add_number_to_stdout(backup_registers->cri_rflags);
	add_char_to_stdout('\n');
}

void registers_handler(BackupRegisters *backup_registers)
{
	RTC_Time time;
	syscall(GET_RTC_TIME_SYSCALL, (uint64_t)&time, 0, 0, 0, 0);
	add_str_to_stdout((char *)"Registers at time: ");
	add_number_to_stdout((uint64_t)time.hours);
	add_char_to_stdout((char)':');
	add_number_to_stdout((uint64_t)time.minutes);
	add_char_to_stdout((char)':');
	add_number_to_stdout((uint64_t)time.seconds);
	add_char_to_stdout((char)'\n');

	// print the backup registers
	add_str_to_stdout((char *)"rax: ");
	add_number_to_stdout(backup_registers->registers.rax);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rbx: ");
	add_number_to_stdout(backup_registers->registers.rbx);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rcx: ");
	add_number_to_stdout(backup_registers->registers.rcx);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rdx: ");
	add_number_to_stdout(backup_registers->registers.rdx);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rsi: ");
	add_number_to_stdout(backup_registers->registers.rsi);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rdi: ");
	add_number_to_stdout(backup_registers->registers.rdi);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rbp: ");
	add_number_to_stdout(backup_registers->registers.rbp);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r8: ");
	add_number_to_stdout(backup_registers->registers.r8);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r9: ");
	add_number_to_stdout(backup_registers->registers.r9);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r10: ");
	add_number_to_stdout(backup_registers->registers.r10);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r11: ");
	add_number_to_stdout(backup_registers->registers.r11);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r12: ");
	add_number_to_stdout(backup_registers->registers.r12);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r13: ");
	add_number_to_stdout(backup_registers->registers.r13);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r14: ");
	add_number_to_stdout(backup_registers->registers.r14);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"r15: ");
	add_number_to_stdout(backup_registers->registers.r15);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rip: ");
	add_number_to_stdout(backup_registers->cri_rip);
	add_char_to_stdout((char)'\n');
	add_str_to_stdout((char *)"rsp: ");
	add_number_to_stdout(backup_registers->cri_rsp);
	add_char_to_stdout((char)'\n');

	if (!graphics_mode)
	{
		redraw();
		if(!running_program) newPrompt();
	}
}

// configures the current line as a prompt, and prints a graphical indicator of that
void newPrompt()
{
	scroll_if_out_of_bounds();
	is_input[current_string] = 1;
	syscall(DRAW_STRING_SYSCALL, (uint64_t)default_prompt, (uint64_t)ColorSchema->prompt, (uint64_t)ColorSchema->background, 0, 0);
	reset_markup();
}

int i = 0;
void restoreContext(uint8_t was_graphic)
{
	running_program = 0;
	if (was_graphic)
	{
		graphics_mode = 0;
		redraw();
	}
	add_char_to_stdout('\n');
	newPrompt();
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

	idle((char *)"idle from restoreContext");
}

// message for debugging purposes
void idle(char *message)
{
	while (1)
	{
		// if (message != 0){
		// 	syscall(DRAW_STRING_SYSCALL, message, ColorSchema->text, ColorSchema->background, 0, 0);
		// }
		_hlt();
	}
}

void home_screen_exit_handler(char event_type, int hold_times, char ascii, char scan_code)
{
	show_home_screen = 0;
}

void home_screen()
{

	syscall(SET_HANDLER_SYSCALL, (uint64_t)KEY_HANDLER, (uint64_t)home_screen_exit_handler, 0, 0, 0);

	Point position = {0};
	int scale = 12;

	int screen_width = getScreenWidth();
	int screen_height = getScreenHeight();

	position.x = (screen_width - MONA_LISA_WIDTH * scale) / 2;
	position.y = (screen_height - MONA_LISA_HEIGHT * scale) / 2;

	drawBitmap((uint32_t *) mona_lisa, MONA_LISA_WIDTH, MONA_LISA_HEIGHT, position, scale);

	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);
	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);

	// center the text
	position.x = 0;
	position.y = 400;

	drawRectangle(ColorSchema->background, screen_width, 140, position);
	position.x += 50;
	position.y += 25;
	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)"Welcome to PinkOS!", (uint64_t)ColorSchema->text, (uint64_t)ColorSchema->background, (uint64_t)&position, 0);

	position.y += 50;

	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)"Press any key to continue", (uint64_t)ColorSchema->text, (uint64_t)ColorSchema->background, (uint64_t)&position, 0);

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
	syscall(SET_SYSTEM_STACK_BASE_SYSCALL, (uint64_t)get_stack_pointer(), 0, 0, 0, 0);
	syscall(SET_CURSOR_LINE_SYSCALL, 1, 0, 0, 0, 0); // evita dibujar la status bar (sí, cambio de idioma cuando se me canta el ogt ** lenguaje!! **)

	home_screen();
	redraw();

	// Setea todos los handlers, para quedar corriendo "en el fondo"
	syscall(SET_HANDLER_SYSCALL, (uint64_t)EXCEPTION_HANDLER, (uint64_t)exception_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, (uint64_t)REGISTERS_HANDLER, (uint64_t)registers_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, (uint64_t)USER_ENVIRONMENT_API_HANDLER, (uint64_t)api_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, (uint64_t)KEY_HANDLER, (uint64_t)key_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, (uint64_t)RTC_HANDLER, (uint64_t)status_bar_handler, 0, 0, 0);
	syscall(SET_HANDLER_SYSCALL, (uint64_t)RESTORE_CONTEXT_HANDLER, (uint64_t)restoreContext, 0, 0, 0);

	current_text_color = ColorSchema->text;
	add_str_to_stdout((char *)"># * This system has a * 90% humor setting * ...\n >#* but only 100% style.\n");
	add_str_to_stdout((char *)"\n >#* Type help for help\n");
	newPrompt();

	idle((char *)"idle from main");
	return 0;
}
