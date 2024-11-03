#include <exceptions/exceptions.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <drivers/registersDriver.h>
#include <processState.h>


void exceptionDispatcher(int exception) {
	saveRegisters();
	callExceptionHandler(exception, getBackupRegisters());
	quitProgram();
}

