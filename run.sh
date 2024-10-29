#!/bin/bash

# Uncomment for PC
# qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

# Uncomment for Mac (known problem: time is 2 hours behind)
qemu-system-x86_64 \
  -hda Image/x64BareBonesImage.qcow2 \
  -m 512 \
  -audiodev coreaudio,id=speaker \
  -machine pcspk-audiodev=speaker
