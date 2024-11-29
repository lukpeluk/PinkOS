#!/bin/bash
# SAMPLE_DATA=0001-sampleDataModule.bin

# printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

# Create a sample data module

for i in {1..5000}
do
    echo "This is sample data.$i" >> 0001-sampleDataModule.bin
done
    dd if=/dev/zero bs=1 count=1 >> 0001-sampleDataModule.bin