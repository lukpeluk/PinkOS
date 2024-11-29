include Makefile.inc

SAMPLE_DATA=0001-sampleDataModule.bin

all: PinkOS sampleDataModule

PinkOS:
	cd PinkOS; make

sampleDataModule:
	# printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)
	./createSampleDataModule.sh
clean:
	cd PinkOS; make clean
	rm -rf *.bin


.PHONY: PinkOS all clean
