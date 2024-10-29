#include <time.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}

void sleep(int milis) {
	int start = ticks;
	// 18.2 ticks per second
	while (ticks - start < milis * 18 / 1000);
}
