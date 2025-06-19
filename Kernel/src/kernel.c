#include <stdint.h>
// #include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>
#include <processManager/processState.h>
#include <interrupts/idtLoader.h>
#include <serial.h>

#include <drivers/videoDriver.h>
#include <drivers/rtcDriver.h>
#include <drivers/serialDriver.h>
#include <drivers/pitDriver.h>
#include <drivers/audioDriver.h>
#include <drivers/mouseDriver.h>

#include <processManager/scheduler.h>
#include <windowManager/windowManager.h>
#include <eventManager/eventManager.h>
#include <processManager/scheduler.h>
#include <programManager/programManager.h>

#include <tests/tests.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const PinkOSAddress = (void*)0x400000; // En decimal 4194304
static void * const sampleDataModuleAddress = (void*)0x500000;

extern void play_sound(uint32_t nFrequence);
extern void stop_sound();

typedef int (*EntryPoint)();



void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary()
{
    
	void * moduleAddresses[] = {
		PinkOSAddress,
		sampleDataModuleAddress // creo que esto habría que sacarlo
	};

	loadModules(&endOfKernelBinary, moduleAddresses);

    ncPrint((const char *)"all modules loaded, clearing bss\n");

    ncPrint((const char *)"bssAddress: ");
    ncPrintHex((uint64_t)bss);
    ncPrint((const char *)"   bssSize: ");
    ncPrintHex((uint64_t)(&endOfKernel - &bss));
    ncPrint((const char *)"   endOfKernel: ");
    ncPrintHex((uint64_t)&endOfKernel);


	clearBSS(&bss, &endOfKernel - &bss);

    ncPrint((const char *)"\nbss cleared, getting stackbase\n");

	void * stackBase = getStackBase();

    ncPrint((const char *)"stack set\n");

	return stackBase;
}


// Ligature always 0 coz it's not supported by the previous format, manually choose notes to ligate
Note * pinkPanther[] = {
    &(Note) {0, 2, 0},
    &(Note) {0, 4, 0},
    &(Note) {0, 8, 0},
    &(Note) {311, 8, 0},
    &(Note) {330, -4, 0},
    &(Note) {0, 8, 0},
    &(Note) {370, 8, 0},
    &(Note) {392, -4, 0},
    &(Note) {0, 8, 0},
    &(Note) {311, 8, 0},
    &(Note) {330, -8, 0},
    &(Note) {370, 8, 0},
    &(Note) {392, -8, 0},
    &(Note) {523, 8, 0},
    &(Note) {494, -8, 0},
    &(Note) {330, 8, 0},
    &(Note) {392, -8, 0},
    &(Note) {494, 8, 0},
    &(Note) {466, 2, 0},
    &(Note) {440, -16, 0},
    &(Note) {392, -16, 0},
    &(Note) {330, -16, 0},
    &(Note) {294, -16, 0},
    &(Note) {330, 2, 0},
    &(Note) {0, 4, 0},
    &(Note) {0, 8, 0},
    &(Note) {311, 4, 0},
    &(Note) {330, -4, 0},
    &(Note) {0, 8, 0},
    &(Note) {370, 8, 0},
    &(Note) {392, -4, 0},
    &(Note) {0, 8, 0},
    &(Note) {311, 8, 0},
    &(Note) {330, -8, 0},
    &(Note) {370, 8, 0},
    &(Note) {392, -8, 0},
    &(Note) {523, 8, 0},
    &(Note) {494, -8, 0},
    &(Note) {392, 8, 0},
    &(Note) {494, -8, 0},
    &(Note) {659, 8, 0},
    &(Note) {622, 1, 0},
    &(Note) {587, 2, 0},
    &(Note) {0, 4, 0},
    &(Note) {0, 8, 0},
    &(Note) {311, 8, 0},
    &(Note) {330, -4, 0},
    &(Note) {0, 8, 0},
    &(Note) {370, 8, 0},
    &(Note) {392, -4, 0},
    &(Note) {0, 8, 0},
    &(Note) {311, 8, 0},
    &(Note) {330, -8, 0},
    &(Note) {370, 8, 0},
    &(Note) {392, -8, 0},
    &(Note) {523, 8, 0},
    &(Note) {494, -8, 0},
    &(Note) {330, 8, 0},
    &(Note) {392, -8, 0},
    &(Note) {494, 8, 0},
    &(Note) {466, 2, 0},
    &(Note) {440, -16, 0},
    &(Note) {392, -16, 0},
    &(Note) {330, -16, 0},
    &(Note) {294, -16, 0},
    &(Note) {330, -4, 0},
    &(Note) {0, 4, 0},
    &(Note) {0, 4, 0},
    &(Note) {659, -8, 0},
    &(Note) {587, 8, 0},
    &(Note) {494, -8, 0},
    &(Note) {440, 8, 0},
    &(Note) {392, -8, 0},
    &(Note) {330, -8, 0},
    &(Note) {466, 16, 0},
    &(Note) {440, -8, 0},
    &(Note) {466, 16, 0},
    &(Note) {440, -8, 0},
    &(Note) {466, 16, 0},
    &(Note) {440, -8, 0},
    &(Note) {466, 16, 0},
    &(Note) {440, -8, 0},
    &(Note) {392, -16, 0},
    &(Note) {330, -16, 0},
    &(Note) {294, -16, 0},
    &(Note) {330, 16, 0},
    &(Note) {330, 16, 0},
    &(Note) {330, 2, 0},
    0,  // Null terminate the song
};

// Test audio, plays duration of 1 second, rest of 1 second and a note for 6 seconds
Note * testAudioNotes[] = {
    &(Note) {440, 4, 0},
    &(Note) {0, 4, 0},
    &(Note) {440, -1, 0},
    0,  // Null terminate the song
};

Note * testChromaticScale[] = {
    &(Note) {262, 1, 0},
    &(Note) {277, 1, 0},
    &(Note) {294, 1, 0},
    &(Note) {311, 1, 0},
    &(Note) {330, 1, 0},
    &(Note) {349, 1, 0},
    &(Note) {370, 1, 0},
    &(Note) {392, 1, 0},
    &(Note) {415, 1, 0},
    &(Note) {440, 1, 0},
    &(Note) {466, 1, 0},
    &(Note) {494, 1, 0},
    &(Note) {523, 1, 0},
    0,
};

Note * testChromaticScaleLigated[] = {
    &(Note) {262, 1, 1},
    &(Note) {277, 1, 1},
    &(Note) {294, 1, 1},
    &(Note) {311, 1, 1},
    &(Note) {330, 1, 1},
    &(Note) {349, 1, 1},
    &(Note) {370, 1, 1},
    &(Note) {392, 1, 1},
    &(Note) {415, 1, 1},
    &(Note) {440, 1, 1},
    &(Note) {466, 1, 1},
    &(Note) {494, 1, 1},
    &(Note) {523, 1, 0},
    0,
};

// El tempo funca joya, el ligado también, el loop lo mismo, y hasta podés cambiar de tema en el medio de una canción y después seguir con la que estabas
void testAudio(){
	// play_sound(440);
	// sleep(1000);
	// stop_sound();

	play_audio(pinkPanther, 1, 140);
    sleep(6000);
    pause_audio();
    // AudioState prev_audio = get_audio_state();

    // play_audio(testAudioNotes, 1, 60);
    // sleep(10000);

    // load_audio_state(prev_audio);
    // resume_audio();
    // sleep(6000);

    // play_audio(testChromaticScale, 1, 120);
    // sleep(10000);
    // play_audio(testChromaticScaleLigated, 1, 120);

    // test rapid notes to see if the delay is registered
    // Note * rapidNotes[] = {
    //     &(Note) {660, 32, 0},
    //     &(Note) {440, 32, 0},
    //     &(Note) {440, 32, 0},
    //     &(Note) {440, 32, 0},
    //     0,
    //     &(Note) {660, 4, 0},
    //     &(Note) {440, 4, 0},
    //     &(Note) {440, 4, 0},
    //     &(Note) {440, 4, 0},
    //     &(Note) {660, 4, 0},
    //     &(Note) {440, 4, 0},
    //     &(Note) {440, 4, 0},
    //     &(Note) {440, 4, 0},
    //     0,
    // };

    // play_audio(rapidNotes, 1, 135);
    // sleep(5000);
    // stop_audio();

    // print the delay
    // drawString((char *)"Delay: ", 0x00ffffff, 0x00000000);
    // drawNumber(get_milliseconds_delayed(), 0x00ffffff, 0x00000000, 0);
    // sleep(5000);
}


int main()
{	
    // ncClear();
	load_idt();
    init_pit();
    init_serial();

    // sleep(1000);

    log_to_serial("I: Usa 'I:' adelante de los mensajes para que el sea informativo");
    log_to_serial("W: Usa 'W:' adelante de los mensajes para que el sea de advertencia");
    log_to_serial("E: Usa 'E:' adelante de los mensajes para que el sea un error");
    log_to_serial("S: Usa 'S:' adelante de los mensajes para que el sea de acierto");

    log_to_serial("I: PinkOS Kernel started");

    ncPrint((const char *)"se llego al main\n");
    // log_to_serial("En el main de kernel\n");
    
	initProcessState();
    initScheduler();
    initVideoDriver();
    initWindowManager();
    initEventManager();
    initProgramManager();
	init_rtc();
	set_timezone(-3);
    init_mouse();

    // test_serial();

    // log_to_serial("I: Kernel binary initialized, clearing BSS");
    // schedulingTest("Scheduler Test");    

    // log_to_serial("I: Testing audio driver");
	// testAudio();
    // log_to_serial("I: Audio test finished");

    // ncPrint((const char *)"ya se termino todo lo de kernel, yendo a userspace\n");

    ((EntryPoint)PinkOSAddress)();

	return 0;
}



