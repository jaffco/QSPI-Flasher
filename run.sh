#!/bin/bash

make clean
make -j$(getconf _NPROCESSORS_ONLN)
make program-dfu

# https://github.com/electro-smith/DaisyWiki/wiki/1.-Setting-Up-Your-Development-Environment#4a-flashing-the-daisy-via-usb
echo "Ignore Error 74 if related to download get_status"