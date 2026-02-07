#!/bin/bash
set -e

echo "====================================="
echo "      QSPI WAV Flasher Tool         "
echo "====================================="
echo ""

# Generate WAV file
echo "Generating 220Hz sine wave WAV file..."
python3 generate_wav.py

# Check if WAV file exists
if [ ! -f "build/sine_220hz.wav" ]; then
    echo "ERROR: WAV file not generated"
    exit 1
fi

echo "Preparing WAV file for QSPI flash..."

# Get WAV file size
WAV_SIZE=$(wc -c < build/sine_220hz.wav | tr -d ' ')
echo "WAV file size: $WAV_SIZE bytes"

# Create flashable binary directly (header + WAV data)
# Magic: "WAVH" = 0x57 0x41 0x56 0x48 (little-endian: 0x48564157)
printf '\x48\x56\x41\x57' > build/wav_flash.bin  # "WAVH" in little-endian

# Write size as little-endian uint32_t
printf "0: %.8x" $WAV_SIZE | sed -E 's/0: (..)(..)(..)(..)/0: \4\3\2\1/' | xxd -r -g0 >> build/wav_flash.bin

# Write checksum placeholder (4 bytes of zeros)
printf '\x00\x00\x00\x00' >> build/wav_flash.bin

# Append WAV data
cat build/sine_220hz.wav >> build/wav_flash.bin

FLASH_SIZE=$(wc -c < build/wav_flash.bin | tr -d ' ')

# Verify final size (should be WAV_SIZE + 12)
EXPECTED_SIZE=$((WAV_SIZE + 12))
if [ "$FLASH_SIZE" -ne "$EXPECTED_SIZE" ]; then
    echo "ERROR: Flash binary size is $FLASH_SIZE, expected $EXPECTED_SIZE"
    exit 1
fi

echo ""
echo "================================"
echo "Flash preparation complete!"
echo "================================"
echo "Generated files:"
echo "  - build/sine_220hz.wav     ($WAV_SIZE bytes)"
echo "  - build/wav_flash.bin      ($FLASH_SIZE bytes) <- Flash this to QSPI"
echo ""
echo "Header contents:"
echo "  Magic:    0x48564157 ('WAVH')"
echo "  Size:     $WAV_SIZE bytes"
echo "  Checksum: 0x00000000 (placeholder)"
echo ""

# Flash to QSPI
echo "Flashing WAV file to QSPI at address 0x90001000..."
dfu-util -a 0 -s 0x90001000 -D build/wav_flash.bin

echo ""
echo "SUCCESS: WAV file flashed to QSPI!"
echo "The file is stored at QSPI address 0x90001000"
echo "It will persist across firmware updates."