#include <drivers/pitDriver.h>

extern void _hlt();

static uint64_t ticks = 0;

void timer_handler() {
	// drawChar(ticks % 10 + '0', 0x00df8090, 0x00000000, 1);
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}

void sleep(uint64_t milis) {
	uint64_t start = ticks;

	// Debug 
	/*
	drawString("Sleeping for ", 0x00df8090, 0x00000000);
	drawNumber(milis, 0x00df8090, 0x00000000, 0);

	drawString("\nStart: ", 0x00df8090, 0x00000000);
	drawNumber(ticks, 0x00df8090, 0x00000000, 0);

	drawString("\nTicks: ", 0x00df8090, 0x00000000);
	drawNumber(ticks, 0x00df8090, 0x00000000, 0);

	drawString("\nEnd:   ", 0x00df8090, 0x00000000);
	drawNumber(start + milis * 18, 0x00df8090, 0x00000000, 0);

	drawString("\nCondition: ", 0x00df8090, 0x00000000);
	drawNumber((start + milis * 18) > ticks, 0x00df8090, 0x00000000, 0);
	*/

	// 18.2 ticks per second
	//while (ticks - start < milis * 19 / 1000){
	int i = 0;
	while (ticks - start < milis * 18 / 1000){
		// drawNumber(i++, 0x00df8090, 0x00000000, 0);
		// drawChar(' ', 0x00df8090, 0x00000000, 1);
		_hlt();
	}
	// drawNumber(ticks - start, 0x00df8090, 0x00000000, 0);
}
