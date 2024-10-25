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
OBJECTS=$(SOURCES:.C=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
STATICCLIBS=

all: $(MODULE)

$(MODULE): $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -T PinkOS.ld _loader.c -o ../$(MODULE) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf asm/*.o *.o

.PHONY: all clean print