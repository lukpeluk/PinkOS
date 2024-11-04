#include <drivers/pitDriver.h>
#include <drivers/registersDriver.h>
#include <drivers/videoDriver.h>

// 18.2 Hz
#define MILLISECONDS_PER_TICK 54.9450549451

extern void _hlt();

static uint64_t ticks = 0;

// Executes jobs that need to supervising something, like the main loop of the audio driver
void timer_handler() {
	ticks++;
	saveRegisters();  // Backup of the registers at any time, in case of wanting them for logging or debugging

	audioLoop();
}

uint64_t ticks_elapsed() {
	return ticks;
}

uint64_t milliseconds_elapsed() {
	return (uint16_t) ticks * MILLISECONDS_PER_TICK;
}

// habría un problema si los ticks hacen overflow
// pero hice la cuenta y eso tardaría 32.496.800.964 años, así que creo que estamos bien

void sleep(uint64_t milis) {
	uint64_t start = milliseconds_elapsed();

	int i = 0;
	while (milliseconds_elapsed() - start < milis){
		_hlt();
	}
}
