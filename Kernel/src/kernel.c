#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>
#include <kernelState.h>

#include <drivers/videoDriver.h>
#include <drivers/rtcDriver.h>
#include <drivers/pitDriver.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const PinkOSAddress = (void*)0x400000;
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
	char buffer[10];

	ncPrint("[x64BareBones]");
	ncNewline();

	ncPrint("CPU Vendor:");
	ncPrint(cpuVendor(buffer));
	ncNewline();

	ncPrint("[Loading modules]");
	ncNewline();

	void * moduleAddresses[] = {
		PinkOSAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	ncPrint("[Done]");
	ncNewline();
	ncNewline();

	ncPrint("[Initializing kernel's binary]");
	ncNewline();

	clearBSS(&bss, &endOfKernel - &bss);

	ncPrint("  text: 0x");
	ncPrintHex((uint64_t)&text);
	ncNewline();
	ncPrint("  rodata: 0x");
	ncPrintHex((uint64_t)&rodata);
	ncNewline();
	ncPrint("  data: 0x");
	ncPrintHex((uint64_t)&data);
	ncNewline();
	ncPrint("  bss: 0x");
	ncPrintHex((uint64_t)&bss);
	ncNewline();

	ncPrint("[Done]");
	ncNewline();
	ncNewline();

	return getStackBase();
}

int main()
{	
	initKernelState();
	init_rtc();
	set_timezone(-3);
	load_idt();

	// while(1){
	// 	drawChar('T', 0x00df8090, 0x00000000);
	// 	sleep(5000);
	// }
	while (1){
		drawRectangle((Point){50, 50}, (Point){500, 500}, 0x00df8090);
		sleep(1000);
		drawRectangle((Point){50, 50}, (Point){500, 500}, 0x00000000);
		sleep(1000);
	}
	drawStringAt("HOLA", 0x00df8090, 0x00000000, (Point){1000, 100});
	drawRectangle((Point){100, 0}, (Point){125, 20}, 0x00df8090);
	// typedef struct {
	// 	uint32_t freq;
	// 	uint32_t duration;
	// 	uint32_t delay;
	// } note_t;

	// note_t notes[] = {
	// 	{0, 500, 250}, {0, 250, 125}, {0, 125, 125}, {311, 125, 125}, 
	// 	{330, 375, 125}, {370, 125, 125}, {392, 375, 125}, {311, 125, 125},
	// 	{330, 250, 125}, {370, 125, 125}, {523, 125, 125}, {494, 125, 125},
	// 	{659, 125, 125}, {587, 1000, 250},
	// 	{0, 500, 250}, {311, 250, 125}, {330, 250, 125}, {370, 250, 125}, 
	// 	{392, 125, 125}, {311, 250, 125}, {392, 125, 125}, {494, 125, 125},
	// 	{466, 125, 125}, {440, 250, 125}, {392, 125, 125}, {440, 125, 125}
	// };
	// const uint32_t song_length = sizeof(notes) / sizeof(notes[0]);

	// for (uint32_t i = 0; i < song_length; i++) {
	// 	beep(notes[i].freq, notes[i].duration);
	// 	sleep(notes[i].delay);
	// }
	// sleep(2000);
	// play_melody();

	ncPrint("[Kernel Main]");
	ncNewline();
	ncPrint("  Sample code module at 0x");
	ncPrintHex((uint64_t)PinkOSAddress);
	ncNewline();
	ncPrint("  Calling the sample code module returned: ");
	ncPrintHex(((EntryPoint)PinkOSAddress)());
	ncNewline();
	ncNewline();

	ncPrint("  Sample data module at 0x");
	ncPrint(" laksjfdlkasjdflkjsadlkf jlkjsdlkfjsaldfkjskd ");
	ncPrintHex((uint64_t)sampleDataModuleAddress);
	ncNewline();
	ncPrint("  Sample data module contents: ");
	ncPrint((char*)sampleDataModuleAddress);
	ncNewline();

	// putPixel(0x00df8090, 0x00000025, 0x00000025);

	ncClear();

	while(1);

	ncPrint("[Finished]");
	return 0;
}



