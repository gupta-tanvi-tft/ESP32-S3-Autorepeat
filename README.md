# ESP32-S3 ES8311 Audio Repeater & Gemini AI Integration

This project is a comprehensive audio solution utilizing the **ESP32-S3** microcontroller and the **Everest Semi ES8311 Audio CODEC**. It functions as a **Full-Duplex Audio Repeater** (Microphone-to-Speaker Passthrough) and includes a Speech-to-Text (STT) integration powered by the **Google Gemini Native Audio API**.

---

## 🌟 Features

- **Full-Duplex Audio**: Concurrent audio input (recording) and output (playback) at 48kHz / 24-bit resolution.
- **Zero-Latency DMA Loopback**: Direct microphone-to-speaker hardware sidetone routing, bypassing CPU overhead.
- **Gemini AI STT Integration**: Captures PCM audio buffers, compresses them (Base64), and streams them directly to the Gemini 2.5 Flash Native Audio API for real-time transcription.
- **High-Fidelity Hardware**: Leverages the ES8311's built-in PGA, High-Pass Filter (removes DC rumble), anti-pop ramping, and Dynamic Range Compression (DRC) limiter.

---

## 🛠 Hardware Architecture

The core of the system relies on the **ESP32-S3 MCU**, **ES8311 CODEC**, and an external **Class-D Power Amplifier** (such as NS4168 or MAX98357).

### Pin Mapping & Wiring Guide

| Signal Group | ES8311 Pin | Function | ESP32-S3 Pin | Notes |
|:---|:---:|:---|:---:|:---|
| **I2C Control** | Pin 1 (`CCLK`) | I2C Clock | **GPIO 1** | 3.3kΩ pull-up to 3.3V |
| | Pin 19 (`CDATA`) | I2C Data | **GPIO 2** | 3.3kΩ pull-up to 3.3V |
| | Pin 20 (`CE`) | I2C Address Select | **GND** | Address = `0x18` |
| **I2S Clocks** | Pin 2 (`MCLK`) | Master Audio Clock | **GPIO 3** | Sourced from ESP32 (12.288 MHz) |
| | Pin 6 (`SCLK`) | I2S Bit Clock | **GPIO 4** | 3.072 MHz @ 48kHz stereo |
| | Pin 8 (`LRCK`) | Word Select / Frame | **GPIO 5** | 48 kHz |
| **I2S Data** | Pin 7 (`ASDOUT`) | ADC Mic Stream Out | **GPIO 6** | To ESP32 I2S RX |
| | Pin 9 (`DSDIN`) | DAC Speaker Stream In | **GPIO 7** | From ESP32 I2S TX |
| **Microphone** | Pin 18 (`MIC1P`) | Mic Pos (+) | Electret (+) | Via 1 µF ceramic cap |
| | Pin 17 (`MIC1N`) | Mic Neg (-) | Electret (-) | Via 1 µF ceramic cap |
| **Speaker** | Pin 12 (`OUTP`) | DAC Analog Out (+) | Power Amp `IN+`| Via 1 µF capacitor |
| | Pin 13 (`OUTN`) | DAC Analog Out (-) | Power Amp `IN-`| Via 1 µF capacitor |

*Note: Ensure proper decoupling capacitors (`1 µF`) are placed on `DACVREF`, `ADCVREF`, and `VMID` for optimal Signal-to-Noise Ratio (SNR).*

---

## 💻 Software Setup (ESP-IDF)

### Prerequisites

Ensure you have the [ESP-IDF (v5.x recommended)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) installed and set up on your machine.

### Configuration

Before flashing, you must configure your Wi-Fi credentials and Gemini API Key. Open `main/stt_client.c` and update the following macros:

```c
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
#define GEMINI_API_KEY "YOUR_GEMINI_API_KEY"
#define GEMINI_MODEL "models/gemini-2.5-flash-native-audio-latest"
```

> **⚠️ Security Warning:** Do **NOT** commit your `GEMINI_API_KEY` to public version control systems like GitHub!

### Building and Flashing

1. Set the target to ESP32-S3:
   ```bash
   idf.py set-target esp32s3
   ```
2. Build the project:
   ```bash
   idf.py build
   ```
3. Flash and monitor the serial output:
   ```bash
   idf.py -p (YOUR_PORT) flash monitor
   ```

---

## 📖 Additional Documentation

For more deep-dives into the specific hardware components and configuration, refer to the included guidebooks:
- [`ES8311_MICROPHONE_GUIDEBOOK.md`](./ES8311_MICROPHONE_GUIDEBOOK.md)
- [`ES8311_SPEAKER_GUIDEBOOK.md`](./ES8311_SPEAKER_GUIDEBOOK.md)
- [`ESP32S3_ES8311_AUDIO_REPEATER.md`](./ESP32S3_ES8311_AUDIO_REPEATER.md)

---

## 🚀 Usage

Once the firmware is flashed and the board is powered:
1. The ESP32-S3 will configure the ES8311 codec over I2C.
2. The device will attempt to connect to the configured Wi-Fi network.
3. Once connected, audio captured from the microphone is streamed via I2S DMA.
4. The audio is immediately passed through to the DAC (Speaker) for zero-latency monitoring.
5. Concurrently, the PCM buffer is encoded to Base64 and sent to the Gemini REST API for real-time transcription. The transcribed text will appear in the serial monitor.

*Tip: To avoid acoustic feedback (howling), keep the microphone physically separated or shielded from the speaker.*
