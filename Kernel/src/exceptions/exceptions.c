#include <exceptions/exceptions.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <drivers/registersDriver.h>
#include <processManager/scheduler.h>
#include <processManager/processState.h>


void exceptionDispatcher(int exception) {
	saveRegisters();
	// callExceptionHandler(exception, getBackupRegisters());
	// quitProgram(getCurrentProcessPID());
	console_log("E: AUCH, se ha producido una excepción: %d", exception);
	// uint16_t cs;
	// __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
	// uint8_t ring = cs & 0x3;
	// console_log("Ring actual: %d\n", (int)ring);
	terminateProcess(getCurrentProcessPID());
}

