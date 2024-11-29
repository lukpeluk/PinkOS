#!/bin/bash
# SAMPLE_DATA=0001-sampleDataModule.bin

# printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

# Create a sample data module
echo "PinkOS v0.6.9 - `date`" >> 0001-sampleDataModule.bin

dd if=/dev/zero bs=1 count=1 >> 0001-sampleDataModule.bin