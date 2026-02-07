# QSPI-Flasher

Flash data directly to Daisy QSPI

This project demonstrates a simple use case- writing a WAV file directly to QSPI and reading it back for use in an audio callback.

## Getting Started

1. Initialize the project: `./init.sh`
2. Use `./run.sh` to flash the WAV file and firmware for reading it back. The script will automatically connect to the Daisy after flashing and log serial output to `log.txt`.

### Files

- `generate_wav.py` - Generates a 220Hz sine wave WAV file
- `flash_wav.sh` - Prepares and flashes the WAV to QSPI
- `src/main.cpp` - Firmware that reads WAV from QSPI and plays it back