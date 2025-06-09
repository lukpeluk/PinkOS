#include <exceptions/exceptions.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <drivers/registersDriver.h>
#include <scheduling/scheduler.h>
#include <processState.h>


void exceptionDispatcher(int exception) {
	saveRegisters();
	// callExceptionHandler(exception, getBackupRegisters());
	// quitProgram(getCurrentProcessPID());
}

