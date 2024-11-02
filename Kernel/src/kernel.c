#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>
#include <processState.h>

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
		sampleDataModuleAddress // creo que esto habría que sacarlo
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

	void * stackBase = getStackBase();
	// setSystemStackBase(stackBase);

	return stackBase;
}

void testScreen(){
	clearScreen(0x00df8090);

	// draw a 7 by 3 px bitmap of a rainbow in the top left corner with 50 pixels of padding and a scale of 3
	uint32_t rainbow[16] = {
		0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x0000FF, 0x4B0082, 0x9400D3,
		0xFF7F00, 0xFFFF00, 0x00FF00, 0x0000FF, 0x4B0082, 0x9400D3, 0xFF0000,
		0xFFFF00, 0x00FF00, 0x0000FF, 0x4B0082, 0x9400D3, 0xFF0000, 0xFF7F00, 
	};
	drawBitmap(rainbow, 7, 3, &(Point){50, 50}, 10);

	Point start = {50, 50};
	Point end = {500, 500};
	drawRectangleBoder(&start, &end, 5, 0x00000000);
	start.x += 10;
	start.y += 10;
	end.x -= 10;
	end.y -= 10;
	drawRectangle(&start, &end, 0x00000000);
}

int main()
{	
	initProcessState();
	init_rtc();
	set_timezone(-3);
	load_idt();

	testScreen();

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



