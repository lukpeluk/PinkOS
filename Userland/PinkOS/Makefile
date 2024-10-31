# include ../Makefile.inc

# MODULE=0000-PinkOS.bin
# SOURCES=$(wildcard [^_]*.c)

# all: $(MODULE)

# $(MODULE): $(SOURCES)
# 	$(GCC) $(GCCFLAGS) -T PinkOS.ld _loader.c $(SOURCES) -o ../$(MODULE)

# clean:
# 	rm -rf *.o

# .PHONY: all clean print

include ../Makefile.inc

MODULE=0000-PinkOS.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
PROGRAM_SOURCES=$(wildcard programs/*.c)
OBJECTS=$(SOURCES:.C=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
STATICCLIBS=

all: $(MODULE)

$(MODULE): $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS) $(PROGRAM_SOURCES)
	$(GCC) $(GCCFLAGS) -T PinkOS.ld _loader.c -I./include -o ../$(MODULE) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS) $(PROGRAM_SOURCES)

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf asm/*.o *.o

.PHONY: all clean print