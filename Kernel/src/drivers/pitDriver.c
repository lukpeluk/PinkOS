#include <drivers/pitDriver.h>
#include <drivers/registersDriver.h>
#include <drivers/videoDriver.h>
#include <drivers/audioDriver.h>
#include <processManager/scheduler.h>
#include <drivers/serialDriver.h>

// 18.2 Hz
#define MILLISECONDS_PER_TICK 54.9450549451

// 1 000 Hz
// #define MILLISECONDS_PER_TICK 1 // 1 millisecond per tick, 1000 ticks per second
// 5 000 Hz
// #define MILLISECONDS_PER_TICK 1.1 // 0.2 milliseconds per tick, 5000 ticks per second

extern void _hlt();

static uint64_t ticks = 0;

// Executes jobs that need to supervising something, like the main loop of the audio driver
void timer_handler() {
	ticks++;
	// saveRegisters(); // Ya no se hace acá, ahora desde assembler cada vez que hay una interrupción se hace el backup de los registros
	log_to_serial("TICK");
	audioLoop(); // Call the audio driver main loop to update the audio stream
	videoLoop(); // Call the video driver main loop to update the video buffer
	schedulerLoop(); // Call the scheduler to switch processes if needed
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

	while (milliseconds_elapsed() - start < milis){
		_hlt();
	}
}
