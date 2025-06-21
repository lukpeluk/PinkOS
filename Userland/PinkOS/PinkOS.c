/* PinkOS.c */
#include <libs/stdpink.h>
#include <libs/serialLib.h>
#include <programs.h>
#include <permissions.h>
#include <libs/events.h>



int init_main()
{
	installProgram(get_program_entry("shell"));
	
	while(1){
		Pid shell_pid = runProgram("shell", "", PRIORITY_NORMAL, 0, 1);

		// ProcessDeathCondition shell_pid_condition = { .pid = shell_pid};
		log_to_serial("PinkOS: Shell started with PID: ");
		log_decimal("PID: ", shell_pid);
		log_decimal("W: MI PID: ", getPID());

		// int a = setWaiting(getPID());
		// log_decimal("PinkOS: Set waiting: ", a);

		// waitForEvent(PROCESS_DEATH_EVENT, &shell_pid, &shell_pid_condition);			// Espera a que el shell muera para volver a iniciarla
		while(1);
		log_to_serial("E: PinkOS: Shell process died, restarting...");
	}
}

int main(){
	static Program init = {
		.command = "init",
		.name = "Init",
		.entry = init_main,
		.permissions = 0xFFFFFFFF & ~DRAWING_PERMISSION, // Para que innit no sea gráfico, y que no tenga una ventana
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