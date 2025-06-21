/* PinkOS.c */
#include <stdint.h>
#include <handlerIds.h>
#include <syscalls/syscallCodes.h>
#include <programs.h>
#include <permissions.h>
#include <ascii.h>
#include <libs/stdpink.h>
#include <libs/graphicsLib.h>
#include <keyboard.h>
#include <libs/audioLib.h>
#include <pictures.h>
#include <colors.h>
#include <libs/events.h>
#include <libs/serialLib.h>

#define PREV_STRING shell_context->current_string > 0 ? shell_context->current_string - 1 : BUFFER_SIZE - 1
#define ADVANCE_INDEX(index, array_size) index = (index + 1) % array_size;
#define DECREASE_INDEX(index, array_size) index = index ? (index - 1) : array_size - 1;

#define IS_ASCII(ascii) ((char)ascii > 0 && (char)ascii < 256)
#define IS_PRINTABLE_CHAR(ascii) ((char)ascii >= 32 && (char)ascii < 255 && (char)ascii != ASCII_DEL)

#define PIPE '|'

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
extern void _hlt();

Colors* ColorSchema = &PinkOSMockupColors;

#define BUFFER_SIZE 500
#define STRING_SIZE 200		 // 199 usable characters and the null termination
#define KEY_REPEAT_ENABLED 1 // 0 for disabling key repeat (no me digan que no es god tener esto)

// #define COMMAND_BUFFER_SIZE 10

static char logo_str[10] = "    PinkOS";
static char time_str[10] = "  00:00:00";
static Point time_position = {950, 5};
static Point logo_position = {10, 5};

static const char *command_not_found_msg = (const char *) ">?Command not found";
static const char *default_prompt = (const char *)" > ";


// STRUCT PARA LOS DATOS DE LA CONSOLA, PARA QUE DISTINTAS INSTANCIAS TENGAN CADA UNO SU PROPIO CONTEXTO
typedef struct ShellContext {
	char buffer[BUFFER_SIZE][STRING_SIZE];  // Buffer circular para los strings de la consola
	uint8_t is_input[BUFFER_SIZE]; 			// Mascara que indica qué líneas del buffer son input del usuario y cuáles output del programa

	int current_string;		// índice de la línea actual del buffer
	int current_position; 	// índice de la posición del cursor en la línea actual
	int oldest_string; 		// índice de la línea más vieja del buffer (para poder scrollear hasta arriba y ver dónde termina el buffer circular)
	int scroll; 			// indica qué línea es la que está en la parte superior de la pantalla

	uint32_t current_text_color;   // Lo cambia el marcado
	int highlighting_text;	       // indica si el texto es resaltado (por el marcado)


	// El primero es el que tendrá el foco (recibe input), y si es el único también es el que manda el output a la consola, si hay dos es el segundo
	Pid running_program_pids[2]; 
	int running_programs; 	// cantidad de programas corriendo (0, 1 o 2 por ahora porque no permitimos pipear más de dos programas)
	uint64_t console_in;    // file descriptors for stdin and stdout
	uint64_t console_out;
	uint64_t pipe;

	Pid threadcito; // Pid of the thread that is outputting the console, used to wake it up when a program is run
} ShellContext;


// ShellContext *shell_context = NULL; // Contexto de la consola, para que cada instancia tenga su propio contexto

ShellContext *getShellContext()
{
	char * pid_str = uint64_to_string(getProcessGroupMain());

	ShellContext *context;
	uint64_t fileId = openFile(pid_str, FILE_READ, FILE_TYPE_RAW_DATA);
	free(pid_str); 

	if(fileId == 0){
		log_to_serial("E: getShellContext: Error opening shell context file");
		return NULL; // Error opening the file, return NULL
	}

	uint32_t bytes_read = readRaw(fileId, (void *)&context, sizeof(uint64_t), 0);
	if(bytes_read != sizeof(uint64_t) || context == NULL)
	{
		log_to_serial("E: getShellContext: Error reading shell context file");
		return NULL;
	}

	return context;
}


void draw_status_bar();
void newPrompt();
void idle(char *message);


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
	ShellContext *shell_context = getShellContext();

	// if enter key is pressed, move to the next string
	if (key == '\n')
	{
		ADVANCE_INDEX(shell_context->current_string, BUFFER_SIZE) // move to the next string

		// if the buffer is full, the oldest string gets overwritten, move the oldest string index to the next string
		// and automatically scroll if a visible line is overwritten
		if (shell_context->current_string == shell_context->oldest_string)
		{
			ADVANCE_INDEX(shell_context->oldest_string, BUFFER_SIZE)

			// TODO: ver si este código está bien acá o es más prolijo abstraer la lógica del scroll
			// igual la única forma de que esto pase es que el buffer sea más chico que la pantalla
			if (shell_context->oldest_string == shell_context->scroll)
			{
				ADVANCE_INDEX(shell_context->scroll, BUFFER_SIZE)
			}
		}

		// reset the current position and terminate the string with 0
		shell_context->current_position = 0;
		shell_context->buffer[shell_context->current_string][shell_context->current_position] = 0;
		shell_context->is_input[shell_context->current_string] = 0; // lines are not input until assigned as such by newPrompt()
	}
	// backspace
	else if (key == 8)
	{
		if (shell_context->current_position > 0)
		{
			shell_context->current_position--;
			shell_context->buffer[shell_context->current_string][shell_context->current_position] = 0;
		}
	}
	else if (IS_PRINTABLE_CHAR(key))
	{
		// checks if the buffer is full, if it is, key presses are ignored
		if (shell_context->current_position == STRING_SIZE - 1)
			return -1;

		// save key to buffer
		shell_context->buffer[shell_context->current_string][shell_context->current_position] = key;
		shell_context->current_position++;
		shell_context->buffer[shell_context->current_string][shell_context->current_position] = 0;
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
		while (number > 0 && i < 11)
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
	ShellContext *shell_context = getShellContext();

	shell_context->current_text_color = ColorSchema->text;
	shell_context->highlighting_text = 0;
}


// recieves a string and updates the current color and highlighting state if the string contains markup at the beginning
// returns the amount of markup chars detected, to avoid printing them
int process_markup(char *string)
{
	ShellContext *shell_context = getShellContext();

	int markup_chars = 2; // 2 is the default amount of markup characters

	if(*string == '=' && *(string + 1) == '=')
		shell_context->highlighting_text = !shell_context->highlighting_text;

	else if(*string == '>' && *(string + 1) == '!')
		shell_context->current_text_color = ColorSchema->error;

	else if(*string == '>' && *(string + 1) == '?')
		shell_context->current_text_color = ColorSchema->warning;

	else if(*string == '>' && *(string + 1) == '+')
		shell_context->current_text_color = ColorSchema->success;

	else if(*string == '>' && *(string + 1) == '.')
		shell_context->current_text_color = ColorSchema->text;
	else if(*string == '>' && *(string + 1) == '#')
		shell_context->current_text_color = ColorSchema->info;
	
	else if(*string == '<' && *(string + 1) == '3'){
		*string = 32;
		*(string + 1) = 169;  
		markup_chars = 0;
	}

	else markup_chars = 0;

	return markup_chars;
}

void draw_status_bar()
{
	int screen_width = 0;
	syscall(GET_SCREEN_WIDTH_SYSCALL, (uint64_t)&screen_width, 0, 0, 0, 0);
	int char_width = getCharWidth();
	time_position.x = getScreenWidth() - (11 * char_width);
	logo_str[2] = 169;

	drawRectangle(ColorSchema->status_bar_background, screen_width, char_width + 10, (Point){0, 0});
	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)logo_str, (uint64_t)ColorSchema->status_bar_text, (uint64_t)ColorSchema->status_bar_background, (uint64_t)&logo_position, 0);
	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)time_str, (uint64_t)ColorSchema->status_bar_text, (uint64_t)ColorSchema->status_bar_background, (uint64_t)&time_position, 0);
}


// configures the current line as a prompt, and prints a graphical indicator of that
void newPrompt()
{
	ShellContext *shell_context = getShellContext();

	scroll_if_out_of_bounds();
	add_char_to_stdout('\n'); // new line before the prompt
	shell_context->is_input[shell_context->current_string] = 1;
	syscall(DRAW_STRING_SYSCALL, (uint64_t)default_prompt, (uint64_t)ColorSchema->prompt, (uint64_t)ColorSchema->background, 0, 0);
	reset_markup();
}


void redraw()
{
	ShellContext *shell_context = getShellContext();

	// clear screen
	clearScreen(ColorSchema->background);
	
	draw_status_bar();
	syscall(SET_CURSOR_LINE_SYSCALL, 2, 0, 0, 0, 0); // evita dibujar la status bar
	reset_markup(); // just in case

	// print the buffer from the scroll position to the current string

	int i = shell_context->scroll ? shell_context->scroll - 1 : BUFFER_SIZE - 1;
	do
	{
		ADVANCE_INDEX(i, BUFFER_SIZE)

		if (shell_context->is_input[i] == 1)
		{
			syscall(DRAW_STRING_SYSCALL, (uint64_t)default_prompt, (uint64_t)ColorSchema->prompt, (uint64_t)ColorSchema->background, 0, 0);
		}
		int j = 0;
		int markup_chars = 0;
		for (; shell_context->buffer[i][j] != 0; j++)
		{
			if(!(i == shell_context->current_string))
				markup_chars = process_markup(shell_context->buffer[i] + j) ; // updates current color and highlighting state if the string contains markup in the current position

			if(markup_chars)
				j += markup_chars-1; // updates the index to avoid printing the markup characters
			else
				syscall(DRAW_CHAR_SYSCALL, (uint64_t)shell_context->buffer[i][j], (uint64_t)shell_context->current_text_color, (uint64_t)(shell_context->highlighting_text ? ColorSchema->highlighted_text_background : ColorSchema->background), 1, 0);
		}
		reset_markup(); // resets the styles in case it was not reset manually

		// print a new line (except in the last string)
		if (i != shell_context->current_string)
			syscall(DRAW_CHAR_SYSCALL, (uint64_t)'\n', (uint64_t)shell_context->current_text_color, (uint64_t)ColorSchema->background, 1, 0);

	} while (i != shell_context->current_string);
}

void clear_buffer()
{
	ShellContext *shell_context = getShellContext();

	// clear the buffer
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		shell_context->buffer[i][0] = 0;
	}
	shell_context->current_position = 0;
	shell_context->current_string = 0;
	shell_context->oldest_string = 0;
	shell_context->scroll = 0;
	reset_markup();
}

void clear_console()
{
	ShellContext *shell_context = getShellContext();

	shell_context->scroll = shell_context->current_string;
	reset_markup();
	redraw();
	//TODO: ver qué se necesita para que todo funque bien
}

void scroll_if_out_of_bounds()
{	
	ShellContext *shell_context = getShellContext();

	// --- AUTO SCROLL ---

	int is_in_boundaries = -1;
	uint8_t needs_redraw = 0;
	do {
		uint64_t cursor_line = 0, cursor_col = 0;
		syscall(GET_CURSOR_LINE_SYSCALL, (uint64_t)&cursor_line, 0, 0, 0, 0);
		syscall(GET_CURSOR_COL_SYSCALL, (uint64_t)&cursor_col, 0, 0, 0, 0);
		
		syscall(IS_CURSOR_IN_BOUNDARIES_SYSCALL, cursor_line, cursor_col + 1, (uint64_t)&is_in_boundaries, 0, 0);

		if(!is_in_boundaries){
			ADVANCE_INDEX(shell_context->scroll, BUFFER_SIZE);
			needs_redraw = 1;
			syscall(SET_CURSOR_LINE_SYSCALL, cursor_line - 1, 0, 0, 0, 0);
		}
	} while(!is_in_boundaries);
	
	if(needs_redraw){
		redraw();
	}
}

void add_char_to_stdin(char character){
	ShellContext *shell_context = getShellContext();

	// Escribir a console_in
	if(!IS_ASCII(character)) return;

	writeFifo(shell_context->console_in, &character, 1);
}

void rm_stdin() {
	ShellContext *shell_context = getShellContext();

	if(!shell_context->console_in) return;
	rmFile(shell_context->console_in);
	shell_context->console_in = 0;
}

void rm_stdout() {
	ShellContext *shell_context = getShellContext();

	if(!shell_context->console_out) return;
	rmFile(shell_context->console_out);
	shell_context->console_out = 0;
}

void rm_pipe() {
	ShellContext *shell_context = getShellContext();

	if(!shell_context->pipe) return;
	rmFile(shell_context->pipe);
	shell_context->pipe = 0;
}

// prints and saves to the buffer
void add_char_to_stdout(char character){
	ShellContext *shell_context = getShellContext();

	if(!IS_ASCII(character)) return;

	// presionar delete en la primera posición no hace nada
	if (character == '\b' && shell_context->current_position == 0)
		return;

	// a new line allways resets the markup
	if (character == '\n')
		reset_markup();

	scroll_if_out_of_bounds();

	int result = save_char_to_buffer(character);

	// if the string limit is reached or the character is unsupported, it simply does nothing
	if (result == -1)
		return;

	syscall(DRAW_CHAR_SYSCALL, (uint64_t)character, (uint64_t)shell_context->current_text_color, (uint64_t)(shell_context->highlighting_text ? ColorSchema->highlighted_text_background : ColorSchema->background), 1, 0);
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
		while (number > 0 && i < 11)
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


void child_death_handler(Pid *pid)
{
	ShellContext *shell_context = getShellContext();

	log_decimal("I: -- USERSPACE HANDLER -- Child process death handler called for PID: ", *pid);

	// if the pid is not the one of the running program, do nothing
	if (*pid != shell_context->running_program_pids[1] && *pid != shell_context->running_program_pids[0]) {
		log_decimal("E: Handler llamado para PID: ", *pid);
		return;
	}

	shell_context->running_programs--;

	// Stdout no se borra, porque puede que haya más cosas para leer de él. Lo borra el thread cuando hay EOF
	if (shell_context->running_program_pids[0] == *pid) {
		// Si murió el primer programa, borramos el stdin
		rm_stdin(); 
		shell_context->running_program_pids[0] = 0;
	} else if (shell_context->running_program_pids[1] == *pid) {
		// Si murió el segundo programa, borramos el pipe
		rm_pipe();
		shell_context->running_program_pids[1] = 0;
	}

	log_to_serial("I: Child process finished: ");
	log_decimal("PID: ", *pid);

	// Si ya no hay más programas corriendo, y tampoco hay cosas para leer de stdout, entonces volvemos al prompt y borramos el pipe
	if (shell_context->running_programs == 0 && shell_context->console_out == 0){
		newPrompt();
	}
}


// Devuelve en command el comando y en args los argumentos
// Y retorna si es nohup o no
int parse_command(int input_line, int index, char * command, char * args){
	log_to_serial("I: Parsing command");

	ShellContext *shell_context = getShellContext();

	int i = index;
	int nohup = 0; // if the program should run in the background (nohup)
	if(shell_context->buffer[input_line][i] == '&') {
		log_to_serial("I: Detected nohup (&) at the beginning of the command");
		nohup = 1; // if the first character is &, run the program in the background
		i++;
	}

	int program_i = 0;
	for (; shell_context->buffer[input_line][i] != ' ' && shell_context->buffer[input_line][i] != 0 && shell_context->buffer[input_line][i] != PIPE; i++)
	{
		command[program_i++] = shell_context->buffer[input_line][i];
	}
	command[program_i] = 0;

	if (shell_context->buffer[input_line][i] == ' '){
		i++;
	}
	// Si no hay argumentos, args queda vacío
	if (shell_context->buffer[input_line][i] == PIPE){
		args[0] = 0;
		return nohup;
	}
	
	int j = 0;
	while (shell_context->buffer[input_line][i] != 0 && shell_context->buffer[input_line][i] != PIPE)
	{
		args[j++] = shell_context->buffer[input_line][i++];
	}

	if (shell_context->buffer[input_line][i] == PIPE && shell_context->buffer[input_line][i-1] == ' '){
		j--;
	}
	args[j] = 0;

	return nohup;
}

// Te dice en qué índice está el siguiente comando si hay un pipe, 0 si no hay pipe
int parse_pipe(int input_line, int index){
	log_to_serial("I: Parsing pipe");

	ShellContext *shell_context = getShellContext();

	int i = index;
	while (shell_context->buffer[input_line][i] != 0 && shell_context->buffer[input_line][i] != PIPE) {
		i++;
	}
	if (shell_context->buffer[input_line][i+1] == ' '){
		i++;
	}
	
	if (shell_context->buffer[input_line][i] == 0) {
		log_to_serial("I: No pipe found");
		return 0; // no pipe found
	}
	
	return i; // return the index of the next command
}


void execute_program(int input_line){
	ShellContext *shell_context = getShellContext();

	// Espacio para los programas y sus argumentos
	char command1[STRING_SIZE];
	char command2[STRING_SIZE];
	char args1[STRING_SIZE];
	char args2[STRING_SIZE];

	int piped_program_index = parse_pipe(input_line, 0); // se fija si hay pipe y en qué índice

	int nohup1 = parse_command(input_line, 0, command1, args1);

	// Get the program entry point
	Program *program1 = get_program_entry(command1);
	int installed = installProgram(program1); // install the program if it was not installed yet

	// if the program is not found, print an error message
	if (program1 == 0 || !installed) {
		// "Command not found"
		add_str_to_stdout((char *)command_not_found_msg);
		newPrompt();
		return;
	}
	
	// Caso hay pipe
	if(piped_program_index){
		int nohup2 = parse_command(input_line, piped_program_index+1, command2, args2);

		Program * program2 = get_program_entry(command2);
		installed = installProgram(program2); // install the program if it was not installed yet

		if (program2 == 0 || !installed) {
			add_str_to_stdout((char *)command_not_found_msg);
			newPrompt();
			return;
		}
		if(nohup1 || nohup2) {
			add_str_to_stdout((char *)">?Error: Cannot run programs with pipes in the background (&)");
			newPrompt();
			return;
		}
	}
	
	// Caso un solo programa y nohup
	if (nohup1) {
		IO_Files io_files = {
			.stdin = 0,
			.stdout = 0,
			.stderr = 0,
		};

		Pid program_pid = runProgram(program1->command, args1, PRIORITY_NORMAL, &io_files, 1);
		if (program_pid == 0) {
			add_str_to_stdout((char *)">?Error running program");
		}
		newPrompt();
		return;
	} else if (piped_program_index == 0) { // caso un solo programa sin pipe
		// Crea los archivos para la entrada y salida estándar del programa
		uint64_t stdin = mkFile("stdin", FILE_TYPE_FIFO, 1024);
		uint64_t stdout = mkFile("stdout", FILE_TYPE_FIFO, 1024);

		// Stderror y stdout mapean al mismo archivo por ahora
		IO_Files io_files = {
			.stdin = stdin,
			.stdout = stdout,
			.stderr = stdout,
		};

		shell_context->console_in = stdin;  // set the console input to the stdin of the program
		shell_context->console_out = stdout; // set the console output to the stdout of the program
		
		Pid program_pid = runProgram(program1->command, args1, PRIORITY_NORMAL, &io_files, 0);
		log_decimal("I: Running program with PID: ", program_pid);

		if (program_pid == 0) {
			add_str_to_stdout((char *)">?Error running program");
			rm_stdin();
			rm_stdout();
			newPrompt();
		} else {
			shell_context->running_programs++;
			shell_context->running_program_pids[0] = program_pid; // save the pid of the running program
			ProcessDeathCondition condition = { .pid = program_pid };
			subscribeToEvent(PROCESS_DEATH_EVENT, (uint64_t)child_death_handler, &condition); // subscribe to the process death event
			wakeProcess(shell_context->threadcito); // thread para el output
		}
		return;
	} else { // caso con pipe
		// Crea los archivos para la entrada y salida estándar del programa
		uint64_t stdin = mkFile("stdin", FILE_TYPE_FIFO, 1024);
		uint64_t stdout = mkFile("stdout", FILE_TYPE_FIFO, 1024);
		shell_context->pipe = mkFile("pipe", FILE_TYPE_FIFO, 1024); // pipe para la comunicación entre los dos programas

		IO_Files io_files1 = {
			.stdin = stdin,
			.stdout = shell_context->pipe,
			.stderr = shell_context->pipe,
		};

		IO_Files io_files2 = {
			.stdin = shell_context->pipe,
			.stdout = stdout,
			.stderr = stdout,
		};

		shell_context->console_in = stdin;
		shell_context->console_out = stdout;

		Pid program_pid1 = runProgram(command1, args1, PRIORITY_NORMAL, &io_files1, 0);
		log_decimal("I: Running first program with PID: ", program_pid1);

		if (program_pid1 == 0) {
			add_str_to_stdout((char *)">?Error running first program");
			rm_stdin();
			rm_stdout();
			rm_pipe();
			newPrompt();
			return;
		}

		Pid program_pid2 = runProgram(command2, args2, PRIORITY_NORMAL, &io_files2, 0);
		log_decimal("I: Running second program with PID: ", program_pid2);

		if (program_pid2 == 0) {
			add_str_to_stdout((char *)">?Error running second program");
			rm_stdin();
			rm_stdout();
			rm_pipe();
			newPrompt();
			return;
		}

		
		shell_context->running_programs += 2;
		
		shell_context->running_program_pids[0] = program_pid1;
		shell_context->running_program_pids[1] = program_pid2;
		
		ProcessDeathCondition condition1 = { .pid = program_pid1 };
		ProcessDeathCondition condition2 = { .pid = program_pid2 };
		
		subscribeToEvent(PROCESS_DEATH_EVENT, (uint64_t)child_death_handler, &condition1);
		subscribeToEvent(PROCESS_DEATH_EVENT, (uint64_t)child_death_handler, &condition2);
		wakeProcess(shell_context->threadcito); // despertar el thread para el output
		return;
	}
}

// void key_handler(char event_type, int hold_times, char ascii, char scan_code)
void key_handler(KeyboardEvent * event)
{
	ShellContext *shell_context = getShellContext();

	char event_type = event->event_type;
	int hold_times = event->hold_times;
	char ascii = event->ascii;
	char scan_code = event->scan_code;
	
	if (event_type != 1 && event_type != 3)  // just register press events (not release or null events)
		return;

	// Si el evento es F5, redibuja la pantalla
	if (scan_code == 0x3F && event_type == 1) // F5 key
	{
		redraw();
		return;
	}

	int is_ctrl_pressed, is_shift_pressed = 0;

	syscall(IS_KEY_PRESSED_SYSCALL, 0x1D, 0, (uint64_t)&is_ctrl_pressed, 0, 0);
	syscall(IS_KEY_PRESSED_SYSCALL, 0x2A, 0, (uint64_t)&is_shift_pressed, 0, 0);


	// --- HOLDING ESC (or pressing ctrl + C) FORCE QUITS ALL RUNNING PROGRAMS (both of them if pipe) ---
	if (((ascii == ASCII_ESC && hold_times == 2) || (is_ctrl_pressed && ascii == 'C')) && shell_context->running_programs)
	{
		killProcess(shell_context->running_program_pids[0]);
		killProcess(shell_context->running_program_pids[1]);
		return;
	} else if (is_ctrl_pressed && ascii == 'c' && shell_context->running_programs)
	{
		killProcess(shell_context->running_program_pids[0]);
		return;
	}

	// Enviar EOF
	if(is_ctrl_pressed && ascii == 'd')
	{
		closeFifoForWriting(shell_context->console_in);
		return;
	}
	

	// --- SCROLL WITH PAGE UP AND PAGE DOWN ---
	if((scan_code == 0x49 || scan_code == 0x51)) // page up or page down
	{
		if((scan_code == 0x49 && event_type == 3))
		{
			if(shell_context->scroll == shell_context->oldest_string) return;  			// don't scroll up if the oldest line is at the top
			if(is_shift_pressed) shell_context->scroll = shell_context->oldest_string;	// with shift + ctrl + up, scroll to the top
			else DECREASE_INDEX(shell_context->scroll, BUFFER_SIZE);
		}
		else if(scan_code == 0x51 && event_type == 3)
		{
			if(shell_context->scroll == shell_context->current_string) return;  			// don't scroll down if the current line is at the bottom
			if(is_shift_pressed) shell_context->scroll = shell_context->current_string;   // whith shift + ctrl + down, scroll to the bottom
			else ADVANCE_INDEX(shell_context->scroll, BUFFER_SIZE);
		}

		redraw();
		return;
	}

	// --- HANDLE SHELL KEYBOARD SHORTCUTS ---
	if(is_ctrl_pressed)
	{
		// --- FONT SIZE ---
		if (ascii == '+' && hold_times == 1)
			syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);  // If ctrl + '+' is pressed, zoom in
		else if (ascii == '-' && hold_times == 1)
			syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);  // If ctrl + '-' is pressed, zoom out

		// --- SCROLL --- 
		else if((scan_code == 0x48 && event_type == 3))
		{
			if(shell_context->scroll == shell_context->oldest_string) return;  			// don't scroll up if the oldest line is at the top
			if(is_shift_pressed) shell_context->scroll = shell_context->oldest_string;	// with shift + ctrl + up, scroll to the top
			else DECREASE_INDEX(shell_context->scroll, BUFFER_SIZE);
		}
		else if(scan_code == 0x50 && event_type == 3)
		{
			if(shell_context->scroll == shell_context->current_string) return;  			// don't scroll down if the current line is at the bottom
			if(is_shift_pressed) shell_context->scroll = shell_context->current_string;   // whith shift + ctrl + down, scroll to the bottom
			else ADVANCE_INDEX(shell_context->scroll, BUFFER_SIZE);
		}
		else if(ascii == 'l')
		{
			shell_context->scroll = shell_context->current_string;
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
		if (shell_context->running_programs){
			if (ascii != ASCII_BS){
				add_char_to_stdout(ascii);
	
				// Si hay un programa corriendo y la entrada es ascii, se lo mando al programa por stdin
				if (ascii)  add_char_to_stdin(ascii);
			}
		}
		else{
			add_char_to_stdout(ascii);
		}
	}

	// If tab pressed, autocomplete the command
	if (ascii == ASCII_HT && !shell_context->running_programs)
	{
		char command[STRING_SIZE] = {0};
		char args[STRING_SIZE] = {0};
		int nohup = parse_command(shell_context->current_string, 0, command, args);
		
		if (command[0] == 0) {
			return;
		}

		// Autocompletar el comando
		Program *autocomplete = searchProgramByPrefix(command);

		if (autocomplete != NULL) {
			// Borra el comando actual
			shell_context->current_position = 0;
			add_str_to_stdout(autocomplete->command);
			add_char_to_stdout(' ');
			redraw(); // redraw to show the new command
		}

		return;
	}
	
	// --- ENTER TO EXECUTE ---
	if (ascii == '\n' && !shell_context->running_programs) {
		log_to_serial("I: Enter pressed, executing program");
		redraw(); // redibuja para que se parsee el marcado
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

// Thread que se encarga de leer de la salida estándar del programa que se está ejecutando
void output_handler(){
	ShellContext *shell_context = getShellContext();

	while (1)
	{
		char buffer[STRING_SIZE] = {0};
		int read = readFifo(shell_context->console_out, buffer, STRING_SIZE-1);

		if (read > 0) {	
			add_str_to_stdout(buffer);
		} else if (read < 0) {
			// EOF: se terminó de leer, me pongo a dormir hasta que me despierten porque hay algo nuevo
			rm_stdout();

			if(!shell_context->running_programs){
				newPrompt(); // si no hay programas corriendo, muestro el prompt
			} 

			setWaiting(getPID());
		} else if (read != 0) {
			log_to_serial("E: Error reading from console_out");
			log_decimal("Read returned: ", read);
		}
	}
}


// message for debugging purposes
void idle(char *message)
{
	while (1) 
		_hlt();
}

void home_screen()
{
	KeyboardCondition condition = {
		.ascii = ' ',   // Condición para que solo espacio te saque de la home screen
	};
	Point position = {0};
	int scale = 12;
	
	int screen_width = getScreenWidth();
	int screen_height = getScreenHeight();
	
	position.x = (screen_width - MONA_LISA_WIDTH * scale) / 2;
	position.y = (screen_height - MONA_LISA_HEIGHT * scale) / 2;

	// drawBitmap((uint32_t *) mona_lisa, MONA_LISA_WIDTH, MONA_LISA_HEIGHT, position, scale);
	clearScreen(ColorSchema->text);
	incFontSize();
	incFontSize();

	// center the text
	position.x = 0;
	position.y = 400;
	
	drawRectangle(ColorSchema->background, screen_width, 140, position);
	position.x += 50;
	position.y += 25;
	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)"Welcome to PinkOS!", (uint64_t)ColorSchema->text, (uint64_t)ColorSchema->background, (uint64_t)&position, 0);
	
	position.y += 50;
	
	syscall(DRAW_STRING_AT_SYSCALL, (uint64_t)"Press space to continue", (uint64_t)ColorSchema->text, (uint64_t)ColorSchema->background, (uint64_t)&position, 0);
	
	syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);
	syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);
	KeyboardEvent event;
	
	waitForEvent(KEY_EVENT, (uint64_t)&event, (uint64_t)&condition);
}


void shell_main(char *args)
{
	ShellContext * shell_context = (ShellContext *)malloc(sizeof(ShellContext));
	if(shell_context == NULL) {
		log_to_serial("E: Failed to allocate memory for shell context");
		return;
	}

	char * pid_str = uint64_to_string(getPID());
	uint64_t fileId = mkFile(pid_str, FILE_TYPE_RAW_DATA, sizeof(uint64_t)); // create the file to store the pointer to the shell context
	free(pid_str);

	if (fileId == 0) {
		log_to_serial("E: Failed to create shell context file");
		free(shell_context);
		return;
	}

	uint32_t bytes_read = writeRaw(fileId, (char *)&shell_context, sizeof(uint64_t), 0); // write the pointer to the shell context
	if (bytes_read != sizeof(uint64_t)) {
		log_to_serial("E: Failed to write shell context pointer to file");
		free(shell_context);
		return;
	}


	log_to_serial("PinkOS shell started");
	syscall(SET_CURSOR_LINE_SYSCALL, 1, 0, 0, 0, 0); // evita dibujar la status bar

	home_screen();
	redraw();

	// Setea todos los handlers, para quedar corriendo "en el fondo"
	subscribeToEvent(KEY_EVENT, (uint64_t)key_handler, 0);
	subscribeToEvent(RTC_EVENT, (uint64_t)status_bar_handler, 0);

	shell_context->threadcito = newThread((void *)output_handler, "", PRIORITY_LOW); // thread for handling output from the console

	shell_context->current_text_color = ColorSchema->text;
	add_str_to_stdout((char *)"># * This system has a * 90% humor setting * ...\n >#* but only 100% style.\n");
	add_str_to_stdout((char *)"\n >#* Type help for help\n");

	// newPrompt(); // No hace falta hacer la prompt acá porque el thread del output al ver que no corre ningún programa ya lo va a hacer 

	wait: setWaiting(getPID());
	goto wait; // por si a algún vivo se le ocurre despertar el proceso de la shell

	return 0;
}
