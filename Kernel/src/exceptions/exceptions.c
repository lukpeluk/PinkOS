#include <exceptions/exceptions.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <drivers/registersDriver.h>
#include <processState.h>


void exceptionDispatcher(int exception) {
	saveRegisters();
	callExceptionHandler(exception, getBackupRegisters());
	quitProgram(0); // 0 means the current process, which is the one that caused the exception
}

