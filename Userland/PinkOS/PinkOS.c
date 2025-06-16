/* PinkOS.c */
#include <libs/stdpink.h>
#include <libs/serialLib.h>
#include <programs.h>
#include <permissions.h>

int init_main()
{
	// static Program shell = {
	// 	.command = "shell",
	// 	.name = "PinkOS Shell",
	// 	.entry = shell_main,
	// 	.permissions = 0xFFFFFFFF,
	// 	.help = "The PinkOS Shell",
	// 	// .description = "Starts the PinkOS shell",
	// };
	// Inicializa el shell
	// syscall(RUN_PROGRAM_SYSCALL, (uint64_t)get_program_entry("snake"), (uint64_t)"1", 0, 0, 0);
	// syscall(RUN_PROGRAM_SYSCALL, (uint64_t)get_program_entry("francis"), (uint64_t)"1", 0, 0, 0);
	// syscall(RUN_PROGRAM_SYSCALL, (uint64_t)get_program_entry("pietra"), (uint64_t)"1", 0, 0, 0);
	log_to_serial("SOY INIT");
	installProgram(get_program_entry("shell"));
	runProgram(get_program_entry("shell"), "", PRIORITY_NORMAL, 0, 1);

	// Si no se pudo inicializar el shell, se queda en un bucle infinito
	while (1)
	{
		_hlt();
	}

	return 0;
}

int main(){
	static Program init = {
		.command = "init",
		.name = "Init",
		.entry = init_main,
		.permissions = 0xFFFFFFFF & ~DRAWING_PERMISSION,
		.help = "The PinkOS init process",
		// .description = "Starts the system and runs the first program (the shell)",
	};

	// Sé que mi PID va a ser 1 ya que es el primer programa que se ejecuta
	// syscall(RUN_PROGRAM_SYSCALL, (uint64_t)&init, (uint64_t)"", 0, 0, 0);
	log_to_serial("PinkOS: Starting init process...");
	installProgram(&init);
	runProgram(&init, "", PRIORITY_HIGH, 0, 0);
	

	idle((char *)"idle from main");
}