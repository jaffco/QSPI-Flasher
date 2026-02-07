#!/usr/bin/env python3
"""
Generate a 220Hz sine wave WAV file for QSPI flashing demo
"""
import numpy as np
import wave
import struct
import os

# Audio parameters
SAMPLE_RATE = 48000  # 48kHz sample rate
FREQUENCY = 220.0    # 220Hz sine wave
DURATION = 1.0       # 1 second
AMPLITUDE = 0.5      # 50% amplitude

# Ensure build directory exists
os.makedirs('build', exist_ok=True)

# Generate sine wave
t = np.linspace(0, DURATION, int(SAMPLE_RATE * DURATION), False)
sine_wave = AMPLITUDE * np.sin(2 * np.pi * FREQUENCY * t)

# Convert to 32-bit float (values should be in range [-1.0, 1.0])
sine_wave_float32 = sine_wave.astype(np.float32)

# Create WAV file with 32-bit float format
with wave.open('build/sine_220hz.wav', 'wb') as wav_file:
    # Set parameters for 32-bit float
    wav_file.setnchannels(1)  # Mono
    wav_file.setsampwidth(4)  # 32-bit (4 bytes per sample)
    wav_file.setframerate(SAMPLE_RATE)
    wav_file.setcomptype('NONE', 'not compressed')

    # Write frames as 32-bit floats
    for sample in sine_wave_float32:
        # Pack as little-endian 32-bit float
        wav_file.writeframes(struct.pack('<f', sample))

print(f"Generated build/sine_220hz.wav: {len(sine_wave_float32)} samples, {DURATION}s duration")
print(f"Format: 32-bit float, {SAMPLE_RATE}Hz, mono")