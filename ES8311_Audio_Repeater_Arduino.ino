/*
 * ESP32-S3 + ES8311 Audio Repeater (Microphone to Speaker)
 * Arduino IDE Compatible Sketch
 */

#include <Arduino.h>
#include <Wire.h>
#include <ESP_I2S.h>

// I2C Pin Mapping
#define I2C_SDA_PIN     2
#define I2C_SCL_PIN     1
#define ES8311_I2C_ADDR 0x18

// I2S Full-Duplex Pin Mapping
#define I2S_MCLK_PIN    3
#define I2S_BCLK_PIN    4
#define I2S_WS_PIN      5
#define I2S_DIN_PIN     6  // Connect to ES8311 ASDOUT (Microphone)
#define I2S_DOUT_PIN    7  // Connect to ES8311 DSDIN (Speaker)

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

    // 1. Reset and precharge
    write_reg(0xFA, 0x01); delay(10);
    write_reg(0x0D, 0x01); delay(25);
    write_reg(0x0D, 0x02);

    // 2. Clocks
    write_reg(0x00, 0x80);
    write_reg(0x01, 0x3F);
    write_reg(0x02, 0x00);
    write_reg(0x03, 0x10);
    write_reg(0x04, 0x10);
    write_reg(0x05, 0x00);

    // 3. Serial format (24-bit I2S)
    write_reg(0x09, 0x00);
    write_reg(0x0A, 0x00);

    // 4. Microphone Front-End
    write_reg(0x14, 0x16); // +18 dB Mic Gain
    write_reg(0x1C, 0x60); // High-pass filter
    write_reg(0x17, 0xBF); // ADC 0 dB

    // 5. Speaker Back-End
    write_reg(0x37, 0x88); // Anti-pop soft ramping
    write_reg(0x32, 0xBF); // Speaker Volume 0 dB
    write_reg(0x34, 0x80); // DRC Peak Limiter
    write_reg(0x35, 0xF0);

    // 6. Power ON
    write_reg(0x0E, 0x00); // Power on ADC
    write_reg(0x13, 0x00); // Line Out mode
    write_reg(0x12, 0x00); // Power on DAC
    write_reg(0x44, 0x00);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting ESP32-S3 Voice Repeater...");

    setup_es8311();

    i2s.setPins(I2S_BCLK_PIN, I2S_WS_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCLK_PIN);
    if (!i2s.begin(I2S_MODE_STD, 48000, I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO)) {
        Serial.println("Failed to start I2S driver!");
        while (1);
    }
    Serial.println("Audio Repeater active! Speak into microphone.");
}

void loop() {
    // Read audio samples from Microphone DMA
    size_t bytes_read = i2s.readBytes((char*)audio_buf, sizeof(audio_buf));
    
    // Immediately write audio samples to Speaker DMA
    if (bytes_read > 0) {
        i2s.write((uint8_t*)audio_buf, bytes_read);
    }
}
