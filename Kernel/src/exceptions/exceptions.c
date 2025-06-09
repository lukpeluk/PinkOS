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
	terminateProcess(getCurrentProcessPID());
}

