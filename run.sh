#!/bin/bash

make clean
source venv/bin/activate
./flash_wav.sh
make -j$(getconf _NPROCESSORS_ONLN)
make program-dfu

# Wait for device to reconnect after flashing
sleep 1

# Auto-detect the USB modem device
DEVICE=$(ls /dev/tty.usbmodem* 2>/dev/null | head -1)
if [ -z "$DEVICE" ]; then
  echo "No USB modem device found. Please ensure the Daisy device is connected."
  exit 1
fi
echo "Connecting to device: $DEVICE"

# Connect to the device and log output
script log.txt screen $DEVICE 115200