#include "../libDaisy/src/daisy_seed.h"
#include <cstring>

using namespace daisy;

static DaisySeed hardware;

// WAV file header structure
struct WavHeader {
    char riff[4];        // "RIFF"
    uint32_t file_size;  // File size - 8
    char wave[4];        // "WAVE"
    char fmt[4];         // "fmt "
    uint32_t fmt_size;   // Format chunk size (16 for PCM)
    uint16_t format;     // Audio format (1 = PCM)
    uint16_t channels;   // Number of channels
    uint32_t sample_rate;// Sample rate
    uint32_t byte_rate;  // Bytes per second
    uint16_t block_align;// Block alignment
    uint16_t bits_per_sample; // Bits per sample
    char data[4];        // "data"
    uint32_t data_size;  // Data size
};

// QSPI flash parameters
static constexpr uint32_t QSPI_BASE_ADDR = 0x90000000;
static constexpr uint32_t WAV_FILE_OFFSET = 0x1000;  // Store WAV at 4KB offset
static constexpr uint32_t CUSTOM_HEADER_SIZE = 12;  // "WAVH" + size + checksum

// Audio playback variables
static WavHeader wav_header;
static uint8_t* wav_data = nullptr;
static uint32_t wav_data_size = 0;
static uint32_t playback_pos = 0;
static uint16_t wav_bits_per_sample = 16;  // Store bits per sample for callback

// Audio callback
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    if (wav_data == nullptr || wav_data_size == 0) {
        // No WAV data, output silence
        for (size_t i = 0; i < size; i++) {
            out[0][i] = 0.0f;
            out[1][i] = 0.0f;
        }
        return;
    }

    for (size_t i = 0; i < size; i++) {
        // Calculate bytes per sample
        uint32_t bytes_per_sample = wav_bits_per_sample / 8;
        
        if (playback_pos >= wav_data_size / bytes_per_sample) {
            playback_pos = 0;  // Loop back to start
        }

        float normalized;
        if (wav_bits_per_sample == 16) {
            // Read 16-bit sample and convert to float (-1.0 to 1.0)
            int16_t sample = *reinterpret_cast<int16_t*>(&wav_data[playback_pos * 2]);
            normalized = static_cast<float>(sample) / 32768.0f;
        } else if (wav_bits_per_sample == 32) {
            // Read 32-bit sample and convert to float (-1.0 to 1.0)
            int32_t sample = *reinterpret_cast<int32_t*>(&wav_data[playback_pos * 4]);
            normalized = static_cast<float>(sample) / 2147483648.0f;
        } else {
            normalized = 0.0f;  // Should not happen
        }

        // Output to both channels
        out[0][i] = normalized;
        out[1][i] = normalized;

        playback_pos++;
    }
}

// Read WAV file from flash and validate header
bool ReadWavFromQspi(WavHeader* header, uint8_t** data, uint32_t* data_size) {
    // Read header from QSPI (skip the 12-byte custom header)
    const WavHeader* qspi_header = reinterpret_cast<const WavHeader*>(
        QSPI_BASE_ADDR + WAV_FILE_OFFSET + CUSTOM_HEADER_SIZE);

    // Invalidate cache for this region
    dsy_dma_invalidate_cache_for_buffer(
        (uint8_t*)(QSPI_BASE_ADDR + WAV_FILE_OFFSET + CUSTOM_HEADER_SIZE),
        sizeof(WavHeader));

    // Copy header
    memcpy(header, qspi_header, sizeof(WavHeader));

    // Debug: Print raw header bytes
    hardware.PrintLine("DEBUG: WAV Header bytes:");
    uint8_t* header_bytes = (uint8_t*)header;
    for (int i = 0; i < 44; i += 4) {
        hardware.PrintLine("  %02X %02X %02X %02X", 
                         header_bytes[i], header_bytes[i+1], 
                         header_bytes[i+2], header_bytes[i+3]);
    }

    // Validate WAV header
    if (memcmp(header->riff, "RIFF", 4) != 0 ||
        memcmp(header->wave, "WAVE", 4) != 0 ||
        memcmp(header->fmt, "fmt ", 4) != 0 ||
        memcmp(header->data, "data", 4) != 0) {
        hardware.PrintLine("ERROR: Invalid WAV header");
        hardware.PrintLine("  RIFF: %.4s (expected RIFF)", header->riff);
        hardware.PrintLine("  WAVE: %.4s (expected WAVE)", header->wave);
        hardware.PrintLine("  fmt:  %.4s (expected fmt )", header->fmt);
        hardware.PrintLine("  data: %.4s (expected data)", header->data);
        return false;
    }

    // Debug: Print header values
    hardware.PrintLine("DEBUG: WAV Header values:");
    hardware.PrintLine("  File size: %d", header->file_size);
    hardware.PrintLine("  Format: %d", header->format);
    hardware.PrintLine("  Channels: %d", header->channels);
    hardware.PrintLine("  Sample rate: %d", header->sample_rate);
    hardware.PrintLine("  Bits per sample: %d", header->bits_per_sample);
    hardware.PrintLine("  Data size: %d", header->data_size);

    if (header->format != 1) {
        hardware.PrintLine("ERROR: Only PCM WAV files supported");
        return false;
    }

    if (header->channels != 1) {
        hardware.PrintLine("ERROR: Only mono WAV files supported");
        return false;
    }

    if (header->bits_per_sample != 16 && header->bits_per_sample != 32) {
        hardware.PrintLine("ERROR: Only 16-bit or 32-bit WAV files supported, got %d bits", header->bits_per_sample);
        return false;
    }

    // Set data pointer and size
    *data = (uint8_t*)(QSPI_BASE_ADDR + WAV_FILE_OFFSET + CUSTOM_HEADER_SIZE + sizeof(WavHeader));
    *data_size = header->data_size;

    // Store bits per sample for the audio callback
    wav_bits_per_sample = header->bits_per_sample;

    // Invalidate cache for data region
    dsy_dma_invalidate_cache_for_buffer(*data, *data_size);

    hardware.PrintLine("WAV file found in QSPI:");
    hardware.PrintLine("  Sample Rate: %d Hz", header->sample_rate);
    hardware.PrintLine("  Channels: %d", header->channels);
    hardware.PrintLine("  Bits/Sample: %d", header->bits_per_sample);
    hardware.PrintLine("  Data Size: %d bytes", header->data_size);

    return true;
}

int main(void) {
    hardware.Init();
    hardware.StartLog(true);

    System::Delay(200);
    hardware.PrintLine("=====================================");
    hardware.PrintLine("      QSPI WAV Player Demo           ");
    hardware.PrintLine("=====================================");
    hardware.PrintLine("");

    // Try to read WAV file from QSPI
    hardware.PrintLine("Reading WAV file from QSPI...");
    if (ReadWavFromQspi(&wav_header, &wav_data, &wav_data_size)) {
        hardware.PrintLine("SUCCESS: WAV file loaded from QSPI!");
        hardware.PrintLine("");

        // Initialize audio
        hardware.PrintLine("Starting audio playback...");
        hardware.StartAudio(AudioCallback);

        hardware.PrintLine("Audio started! Playing WAV in loop.");
        hardware.PrintLine("WAV stored at QSPI address 0x%08X", WAV_FILE_OFFSET);

        // Main loop - just blink LED
        while (true) {
            static bool ledState = false;
            ledState = !ledState;
            hardware.SetLed(ledState);
            System::Delay(500);
        }
    } else {
        hardware.PrintLine("ERROR: Failed to read WAV file from QSPI");
        hardware.PrintLine("Make sure to flash a WAV file first using:");
        hardware.PrintLine("  python3 generate_wav.py && ./flash_wav.sh");

        // Still start audio but with silence
        hardware.StartAudio(AudioCallback);

        while (true) {
            static bool ledState = false;
            ledState = !ledState;
            hardware.SetLed(ledState);
            System::Delay(100);  // Faster blink to indicate error
        }
    }

    return 0;
}