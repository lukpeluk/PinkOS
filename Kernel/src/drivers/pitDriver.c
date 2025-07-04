#include <drivers/pitDriver.h>
#include <drivers/registersDriver.h>
#include <drivers/videoDriver.h>
#include <drivers/audioDriver.h>
#include <processManager/scheduler.h>
#include <drivers/serialDriver.h>
#include <eventManager/eventManager.h>
#include <interrupts/keyboardInterrupt.h>

// 18.2 Hz
// #define MILLISECONDS_PER_TICK 54.9450549451
// 50 Hz
#define MILLISECONDS_PER_TICK 20 // 20 milliseconds per tick, 50 ticks per second 

// 1 000 Hz
// #define MILLISECONDS_PER_TICK 1 // 1 millisecond per tick, 1000 ticks per second
// 5 000 Hz
// #define MILLISECONDS_PER_TICK 1.1 // 0.2 milliseconds per tick, 5000 ticks per second

extern void _hlt();

static uint64_t ticks = 0;

// Executes jobs that need to supervising something, like the main loop of the audio driver
void timer_handler() {
	// log_to_serial("TICK");
	ticks++;
	handleSleep(milliseconds_elapsed()); // Handle sleep events based on the elapsed time
	
	// audioLoop(); // Call the audio driver main loop to update the audio stream
	checkOverlayDelay();
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
	log_to_serial("E: MAL MAL MAL, NO SE TIENE QUE USAR MAS ESTE SLEEP, DEPRECADISIMO");
	return;

	uint64_t start = milliseconds_elapsed();

	while (milliseconds_elapsed() - start < milis){
		_hlt();
	}
}
