#include <exceptions/exceptions.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <drivers/registersDriver.h>
#include <processState.h>




static void zero_division();
static void invalid_opcode();

void exceptionDispatcher(int exception) {
	saveRegisters();
	callExceptionHandler(exception, getBackupRegisters());
	quitProgram();
}

// De momento no parece que tengamos la necesidad de llamar a estos handlers, pero los dejo por si acaso

static void zero_division() {
	// Handler para manejar excepcíon
}

static void invalid_opcode() {
	// Handler para manejar excepcíon
}