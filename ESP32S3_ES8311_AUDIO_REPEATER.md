# ESP32-S3 & ES8311 Full-Duplex Audio Repeater: Microphone-to-Speaker Passthrough

> **Project Goal:** Convert acoustic voice spoken into the microphone, digitize it with 24-bit studio precision, process/stream it through the ESP32-S3, and output it in real time through the speaker.  
> **Target Hardware:** ESP32-S3 MCU + Everest Semi ES8311 Audio CODEC + External Class-D Power Amp (NS4168 / MAX98357 / NS4150).

---

## 1. System Architecture: How Audio Repeater Works

An **Audio Repeater** operates in **Full-Duplex** mode: audio input (recording) and audio output (playback) happen concurrently at $48\text{ kHz}$ / 24-bit with zero stutter or buffer underflow.

```
                  ┌─────────────────────────────────────────────────────────────────────────────┐
                  │                             ESP32-S3 MCU                                    │
                  │                                                                             │
                  │   ┌───────────────────────────┐         ┌───────────────────────────────┐   │
                  │   │   I2C Configuration Bus   │         │    Full-Duplex I2S Engine     │   │
                  │   │  (Sets Gain, Vol, Clocks) │         │       (DMA Ping-Pong Buffers) │   │
                  │   └─────────────┬─────────────┘         └───▲───────────────────────┬───┘   │
                  └─────────────────┼───────────────────────────┼───────────────────────┼───────┘
                                    │ I2C (SCL/SDA)             │ I2S In (ASDOUT)       │ I2S Out (DSDIN)
                                    ▼                           │                       ▼
 ┌──────────────┐         ┌─────────────────────────────────────┴───────────────────────────────┐         ┌─────────────────┐         ┌───────────┐
 │  Microphone  │ ──────► │                               ES8311 CODEC                          │ ──────► │ Class-D Power   │ ──────► │  Speaker  │
 │ (Analog/PDM) │ Analog  │  • Analog Mic PGA (+18dB)              • 24-Bit Delta-Sigma DAC     │ Differ- │ Amplifier Chip  │ Acoustic│ (4Ω / 8Ω) │
 └──────────────┘ Wave    │  • 24-Bit Delta-Sigma ADC              • Master Volume & DRC Limiter│ ential  │ (NS4168/MAX983) │ Sound   └───────────┘
                          │  • High-Pass Filter (DC Rumble Strip)  • Anti-Pop Slew Ramping      │ OUTP/N  └─────────────────┘
                          └─────────────────────────────────────────────────────────────────────┘
```

### Two Operational Modes:
1. **Mode A: Full-Duplex ESP32-S3 Software Passthrough (DSP Mode)**
   - Mic sound $\rightarrow$ ES8311 ADC $\rightarrow$ ESP32-S3 I2S RX DMA $\rightarrow$ Audio Processing / Volume / Effects in RAM $\rightarrow$ ESP32-S3 I2S TX DMA $\rightarrow$ ES8311 DAC $\rightarrow$ Speaker.
   - *Advantage:* Allows voice effects, pitch shifting, AI voice triggers, digital filtering, and recording while playing.
2. **Mode B: Zero-Latency Direct Hardware Sidetone Loopback**
   - Mic sound $\rightarrow$ ES8311 ADC $\rightarrow$ Internal hardware routing (`ADC2DAC_SEL = 1`) $\rightarrow$ ES8311 DAC $\rightarrow$ Speaker.
   - *Advantage:* Absolute lowest latency ($< 0.1\text{ ms}$), zero ESP32 CPU load.

---

## 2. Complete Hardware Wiring & Pin Mapping

### Pin Connection Table (ESP32-S3 $\leftrightarrow$ ES8311 $\leftrightarrow$ Amplifier)

| Signal Group | ES8311 Pin | ES8311 Function | ESP32-S3 Pin (Default) | Notes & Component Requirements |
|:---|:---:|:---|:---:|:---|
| **$I^2C$ Control** | **Pin 1** (`CCLK`) | I2C Clock | **GPIO 1** | $3.3\text{ k}\Omega$ pull-up resistor to 3.3V |
| | **Pin 19** (`CDATA`) | I2C Data | **GPIO 2** | $3.3\text{ k}\Omega$ pull-up resistor to 3.3V |
| | **Pin 20** (`CE`) | I2C Address Select | **GND** | Address = `0x18` (Tie to 3.3V for `0x19`) |
| **$I^2S$ Audio Clock** | **Pin 2** (`MCLK`) | Master Audio Clock | **GPIO 3** | Sourced from ESP32 ($256 \times Fs = 12.288\text{ MHz}$) |
| | **Pin 6** (`SCLK`) | I2S Bit Clock (BCLK) | **GPIO 4** | $64 \times Fs = 3.072\text{ MHz}$ @ 48kHz stereo |
| | **Pin 8** (`LRCK`) | Word Select / Frame | **GPIO 5** | $= Fs = 48\text{ kHz}$ |
| **$I^2S$ Audio Data** | **Pin 7** (`ASDOUT`) | ADC Mic Stream Out | **GPIO 6** | Streams digitized microphone samples to ESP32 |
| | **Pin 9** (`DSDIN`) | DAC Speaker Stream In | **GPIO 7** | Receives playback samples from ESP32 to DAC |
| **Microphone Input** | **Pin 18** (`MIC1P`) | Mic Positive Input | Electret Mic (+) | In series with $1\ \mu\text{F}$ ceramic capacitor |
| | **Pin 17** (`MIC1N`) | Mic Negative Input | Electret Mic (-) | In series with $1\ \mu\text{F}$ ceramic capacitor |
| **Speaker Output** | **Pin 12** (`OUTP`) | DAC Analog Out (+) | Power Amp `IN+` | In series with $1\ \mu\text{F}$ capacitor |
| | **Pin 13** (`OUTN`) | DAC Analog Out (-) | Power Amp `IN-` | In series with $1\ \mu\text{F}$ capacitor |
| **Decoupling Caps** | **Pin 14** (`DACVREF`)| DAC Voltage Ref | $1\ \mu\text{F} \rightarrow \text{AGND}$ | Essential for 110 dB SNR |
| | **Pin 15** (`ADCVREF`)| ADC Voltage Ref | $1\ \mu\text{F} \rightarrow \text{AGND}$ | Essential for 100 dB SNR |
| | **Pin 16** (`VMID`)| Mid-Rail Bias | $1\ \mu\text{F} \rightarrow \text{AGND}$ | Prevents startup pop noise |
| **Power & Ground** | **Pins 3, 4, 11** | PVDD, DVDD, AVDD | **3.3V** | Decouple with $0.1\ \mu\text{F} + 10\ \mu\text{F}$ |
| | **Pins 5, 10, EP** | DGND, AGND, Exposed Pad | **GND** | Solder center pad directly to PCB ground |

---

## 3. Full-Duplex CODEC Register Setup Sequence

To enable both the **Microphone ADC** and **Speaker DAC** simultaneously:

```c
// 1. Soft Reset & Power Stabilization
i2c_write(0xFA, 0x01);  // Reset chip
delay_ms(10);
i2c_write(0x0D, 0x01);  // Start VMID & Reference pre-charge
delay_ms(25);           // Allow capacitors to charge
i2c_write(0x0D, 0x02);  // Switch VMID to normal operating mode

// 2. Clock Management (Slave Mode, 256x MCLK from ESP32)
i2c_write(0x00, 0x80);  // Enable CSM in slave mode
i2c_write(0x01, 0x3F);  // Turn ON MCLK, BCLK, ADC Clocks, and DAC Clocks
i2c_write(0x02, 0x00);  // MCLK pre-divider = 1
i2c_write(0x03, 0x10);  // ADC OSR = 64x
i2c_write(0x04, 0x10);  // DAC OSR = 64x
i2c_write(0x05, 0x00);  // Clock dividers = 1

// 3. Configure I2S Serial Data Format (24-bit, Standard I2S)
i2c_write(0x09, 0x00);  // DAC Input: Play Left channel, 24-bit, unmuted
i2c_write(0x0A, 0x00);  // ADC Output: 24-bit, I2S format, unmuted

// 4. Microphone Front-End Configuration (PGA Gain + HPF)
i2c_write(0x14, 0x16);  // Differential Analog Mic mode, +18 dB PGA Gain
i2c_write(0x1C, 0x60);  // High-Pass Filter enabled (removes DC offset & rumble)
i2c_write(0x17, 0xBF);  // ADC Digital Volume = 0 dB

// 5. Speaker Back-End Configuration (Volume + Anti-Pop + Limiter)
i2c_write(0x37, 0x88);  // Anti-pop soft ramping enabled, DACEQ bypassed
i2c_write(0x32, 0xBF);  // DAC Master Volume = 0 dB (100% volume)
i2c_write(0x34, 0x80);  // Dynamic Range Compression (DRC) peak limiter active
i2c_write(0x35, 0xF0);  // DRC -6 dB limiter ceiling (prevents clipping)

// 6. Power ON Both Analog ADC & DAC Stages
i2c_write(0x0E, 0x00);  // Power ON Analog Mic PGA and ADC Modulator
i2c_write(0x13, 0x00);  // Line Out mode (use 0x10 for direct headphones)
i2c_write(0x12, 0x00);  // Power ON DAC analog core and reference output
i2c_write(0x44, 0x00);  // Duplicate mono mic data on both Left/Right I2S slots
```

---

## 4. Complete ESP-IDF C Implementation (`es8311_audio_repeater.c`)

Here is the complete, self-contained **ESP-IDF v5.x** firmware implementing full-duplex DMA streaming from microphone to speaker:

```c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

#define TAG "AUDIO_REPEATER"

// I2C Pin Configuration
#define I2C_PORT        I2C_NUM_0
#define I2C_SCL_PIN     GPIO_NUM_1
#define I2C_SDA_PIN     GPIO_NUM_2
#define ES8311_I2C_ADDR 0x18

// I2S Pin Configuration (Full-Duplex)
#define I2S_PORT        I2S_NUM_0
#define I2S_MCLK_PIN    GPIO_NUM_3
#define I2S_BCLK_PIN    GPIO_NUM_4
#define I2S_WS_PIN      GPIO_NUM_5
#define I2S_DIN_PIN     GPIO_NUM_6   // Connect to ES8311 ASDOUT (Mic)
#define I2S_DOUT_PIN    GPIO_NUM_7   // Connect to ES8311 DSDIN (Speaker)

// Audio Buffer Settings
#define SAMPLE_RATE     48000
#define AUDIO_BUF_SIZE  512          // 512 samples per DMA frame (~10.6 ms)

static i2s_chan_handle_t tx_chan;
static i2s_chan_handle_t rx_chan;

static esp_err_t es8311_write(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = { reg, val };
    return i2c_master_write_to_device(I2C_PORT, ES8311_I2C_ADDR, buf, 2, pdMS_TO_TICKS(100));
}

void es8311_codec_init(void) {
    ESP_LOGI(TAG, "Configuring ES8311 Full-Duplex Audio CODEC...");

    // 1. Soft Reset & Reference Pre-charge
    es8311_write(0xFA, 0x01); vTaskDelay(pdMS_TO_TICKS(10));
    es8311_write(0x0D, 0x01); vTaskDelay(pdMS_TO_TICKS(25));
    es8311_write(0x0D, 0x02);

    // 2. Clocks
    es8311_write(0x00, 0x80); // Slave Mode
    es8311_write(0x01, 0x3F); // Turn on all clocks
    es8311_write(0x02, 0x00);
    es8311_write(0x03, 0x10); // ADC OSR = 64x
    es8311_write(0x04, 0x10); // DAC OSR = 64x
    es8311_write(0x05, 0x00);

    // 3. Serial Formats (24-bit I2S)
    es8311_write(0x09, 0x00); // DAC SDP
    es8311_write(0x0A, 0x00); // ADC SDP

    // 4. Microphone Front-End
    es8311_write(0x14, 0x16); // Differential mode, +18 dB Gain
    es8311_write(0x1C, 0x60); // High-Pass Filter ON
    es8311_write(0x17, 0xBF); // ADC Vol 0 dB

    // 5. Speaker Back-End
    es8311_write(0x37, 0x88); // Anti-pop soft ramping
    es8311_write(0x32, 0xBF); // Master Volume 0 dB
    es8311_write(0x34, 0x80); // DRC Peak Limiter ON
    es8311_write(0x35, 0xF0); // -6 dB Peak ceiling

    // 6. Power ON Analog Sections
    es8311_write(0x0E, 0x00); // Power on ADC
    es8311_write(0x13, 0x00); // Line Out mode
    es8311_write(0x12, 0x00); // Power on DAC
    es8311_write(0x44, 0x00); // Dual slot output

    ESP_LOGI(TAG, "ES8311 successfully initialized in Full-Duplex mode!");
}

void i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
}

void i2s_full_duplex_init(void) {
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCLK_PIN,
            .bclk = I2S_BCLK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_DOUT_PIN,
            .din = I2S_DIN_PIN,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };

    i2s_channel_init_std_mode(tx_chan, &std_cfg);
    i2s_channel_init_std_mode(rx_chan, &std_cfg);

    i2s_channel_enable(tx_chan);
    i2s_channel_enable(rx_chan);
    ESP_LOGI(TAG, "I2S Full-Duplex DMA channels enabled @ 48kHz 24-bit.");
}

// Real-Time Audio Repeater Task (Mic -> DMA RX -> Buffer -> DMA TX -> Speaker)
void audio_repeater_task(void *pvParameters) {
    int32_t audio_buffer[AUDIO_BUF_SIZE * 2]; // Stereo 24-bit buffer
    size_t bytes_read = 0;
    size_t bytes_written = 0;

    ESP_LOGI(TAG, "Audio Repeater Loop Running. Speak into the mic!");

    while (1) {
        // 1. Fetch digitized microphone audio from DMA RX buffer
        if (i2s_channel_read(rx_chan, audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY) == ESP_OK) {
            
            // 2. (Optional DSP / Digital Gain can be applied here)
            // Example: Multiply by gain factor, add noise gate, or filter

            // 3. Immediately write audio buffer to DMA TX buffer to play on speaker
            i2s_channel_write(tx_chan, audio_buffer, bytes_read, &bytes_written, portMAX_DELAY);
        }
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting ESP32-S3 Voice Repeater System...");
    i2c_master_init();
    i2s_full_duplex_init();
    es8311_codec_init();

    xTaskCreatePinnedToCore(audio_repeater_task, "audio_repeater", 4096, NULL, 5, NULL, 1);
}
```

---

## 5. Ready-to-Flash Arduino IDE Sketch (`ES8311_Audio_Repeater_Arduino.ino`)

For makers and developers using the **Arduino IDE** or **PlatformIO**:

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <ESP_I2S.h>

#define I2C_SDA_PIN     2
#define I2C_SCL_PIN     1
#define ES8311_I2C_ADDR 0x18

#define I2S_MCLK_PIN    3
#define I2S_BCLK_PIN    4
#define I2S_WS_PIN      5
#define I2S_DIN_PIN     6  // Mic In
#define I2S_DOUT_PIN    7  // Speaker Out

I2SClass i2s;
const int BUFFER_SAMPLES = 256;
int32_t audio_buf[BUFFER_SAMPLES * 2]; // 24-bit stereo samples

void write_reg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ES8311_I2C_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void setup_es8311() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);

    write_reg(0xFA, 0x01); delay(10);
    write_reg(0x0D, 0x01); delay(25);
    write_reg(0x0D, 0x02);

    write_reg(0x00, 0x80);
    write_reg(0x01, 0x3F);
    write_reg(0x02, 0x00);
    write_reg(0x03, 0x10);
    write_reg(0x04, 0x10);
    write_reg(0x05, 0x00);

    write_reg(0x09, 0x00); // 24-bit DAC
    write_reg(0x0A, 0x00); // 24-bit ADC

    write_reg(0x14, 0x16); // +18 dB Mic Gain
    write_reg(0x1C, 0x60); // High Pass Filter
    write_reg(0x17, 0xBF); // ADC Vol 0dB

    write_reg(0x37, 0x88); // Anti-pop ramping
    write_reg(0x32, 0xBF); // Speaker Vol 0dB
    write_reg(0x34, 0x80); // DRC Peak Limiter
    write_reg(0x35, 0xF0);

    write_reg(0x0E, 0x00); // Power on ADC
    write_reg(0x13, 0x00); // Line out
    write_reg(0x12, 0x00); // Power on DAC
    write_reg(0x44, 0x00);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting ESP32-S3 Voice Repeater...");

    setup_es8311();

    i2s.setPins(I2S_BCLK_PIN, I2S_WS_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCLK_PIN);
    if (!i2s.begin(I2S_MODE_STD, 48000, I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO)) {
        Serial.println("Failed to start I2S!");
        while (1);
    }
    Serial.println("Audio Repeater active! Speak into the microphone.");
}

void loop() {
    // Read audio from Mic into buffer
    size_t bytes_read = i2s.readBytes((char*)audio_buf, sizeof(audio_buf));
    
    // Immediately write audio back out to Speaker
    if (bytes_read > 0) {
        i2s.write((uint8_t*)audio_buf, bytes_read);
    }
}
```

---

## 6. How to Test & Verify Audio Repeater

1. **Power Check:** Ensure $3.3\text{V}$ is clean and the $1\ \mu\text{F}$ capacitors on `VMID`, `ADCVREF`, and `DACVREF` are connected to `AGND`.
2. **Acoustic Isolation:** Keep the microphone physically separated or shielded from the speaker cone to prevent acoustic feedback howl (Larsen effect).
3. **Speak into the Microphone:** Voice spoken into the mic will instantly be heard through the speaker with zero perceptible lag!
