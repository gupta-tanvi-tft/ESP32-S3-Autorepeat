/*
 * ES8311 + ES7210 Audio Repeater for Waveshare ESP32-S3-AUDIO-Board
 *
 * Records 3 seconds of audio from the ES7210 mic ADC, then plays it back
 * through the ES8311 DAC + NS4150B speaker amplifier.
 *
 * Architecture:
 *   - TCA9555 GPIO expander (0x20): Pin EXIO8 = PA_EN (amplifier enable)
 *   - ES7210 quad-ADC (0x40):  Mic input via I2S DIN
 *   - ES8311 codec (0x18):     Speaker output via I2S DOUT
 *   - Shared I2S bus: ESP32 is I2S master, both codecs are slaves
 *
 * Waveshare ESP32-S3-AUDIO-Board Pin Mapping:
 *   I2C SDA  = GPIO 11       I2S MCLK = GPIO 12
 *   I2C SCL  = GPIO 10       I2S BCLK = GPIO 13
 *                             I2S LRCK = GPIO 14
 *                             I2S DIN  = GPIO 15  (ES7210 -> ESP32)
 *                             I2S DOUT = GPIO 16  (ESP32 -> ES8311)
 */

#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "led_strip.h"
#include "stt_client.h"
#include "tts_client.h"

#define LED_STRIP_GPIO GPIO_NUM_38
#define LED_STRIP_NUM  7
static led_strip_handle_t led_strip = NULL;

/* ─── I2S handles ─── */
static i2s_chan_handle_t rx_chan = NULL;  /* Mic input */
static i2s_chan_handle_t tx_chan = NULL;  /* Speaker output */
static bool ai_spoke = false;

static void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    if (!led_strip) return;
    led_strip_clear(led_strip);
    for (int i = 0; i < LED_STRIP_NUM; i++) {
        led_strip_set_pixel(led_strip, i, r, g, b);
    }
    led_strip_refresh(led_strip);
}

/* ─── STT Result Callback: Triggers AI TTS Repeat ─── */
static void on_stt_result(const char *transcribed_text) {
    if (!transcribed_text || strlen(transcribed_text) == 0) return;
    if (strstr(transcribed_text, "[NO SPEECH]") != NULL) return;
    ESP_LOGI("AUDIO_REPEATER", ">> [AI REPEATER] Repeating verbatim: \"%s\"", transcribed_text);
    set_led_color(0, 50, 50); // Cyan when AI is speaking
    tts_synthesize_and_play(transcribed_text, tx_chan);
    ai_spoke = true;
}

static const char *TAG = "AUDIO_REPEATER";

/* ─── Waveshare ESP32-S3-AUDIO-Board Pin Definitions ─── */
#define I2C_PORT        I2C_NUM_0
#define I2C_SDA_PIN     GPIO_NUM_11
#define I2C_SCL_PIN     GPIO_NUM_10
#define I2C_CLK_SPEED   100000

#define I2S_MCLK_PIN    GPIO_NUM_12
#define I2S_BCLK_PIN    GPIO_NUM_13
#define I2S_LRCK_PIN    GPIO_NUM_14
#define I2S_DIN_PIN     GPIO_NUM_15   /* ES7210 mic data -> ESP */
#define I2S_DOUT_PIN    GPIO_NUM_16   /* ESP -> ES8311 speaker */

/* ─── Audio Configuration ─── */
#define SAMPLE_RATE     16000          /* 16 kHz — good for voice */
#define BITS_PER_SAMPLE 16
#define CHANNELS        2              /* Stereo */
#define RECORD_SECONDS  4
#define BYTES_PER_SAMPLE (BITS_PER_SAMPLE / 8)
#define RECORD_SIZE     (SAMPLE_RATE * BYTES_PER_SAMPLE * CHANNELS * RECORD_SECONDS)

/* ─── I2C Device Addresses ─── */
#define ES8311_ADDR     0x18           /* DAC (speaker) */
#define ES7210_ADDR     0x40           /* ADC (mic) */
#define TCA9555_ADDR    0x20           /* GPIO expander */

/* ─── TCA9555 Register Definitions ─── */
#define TCA9555_OUTPUT0     0x02
#define TCA9555_OUTPUT1     0x03
#define TCA9555_CONFIG0     0x06
#define TCA9555_CONFIG1     0x07
#define TCA9555_PA_EN_BIT   0  /* EXIO8 = bit 0 of port 1 */

/* ─── ES7210 Register Definitions ─── */
#define ES7210_RESET        0x00
#define ES7210_CLOCK1       0x01
#define ES7210_CLOCK2       0x02
#define ES7210_MAIN_CLK     0x03
#define ES7210_LRCK_DIV_H   0x04
#define ES7210_LRCK_DIV_L   0x05
#define ES7210_POWER_DOWN   0x06
#define ES7210_OSR          0x07
#define ES7210_MODE_CFG     0x08
#define ES7210_TDM_CFG      0x09
#define ES7210_SDP_CFG      0x11
#define ES7210_ADC12_HPF1   0x1B
#define ES7210_ADC12_HPF2   0x1C
#define ES7210_ADC34_HPF1   0x1D
#define ES7210_ADC34_HPF2   0x1E
#define ES7210_ADC1_GAIN    0x43
#define ES7210_ADC2_GAIN    0x44
#define ES7210_ADC3_GAIN    0x45
#define ES7210_ADC4_GAIN    0x46
#define ES7210_MIC12_POWER  0x4B
#define ES7210_MIC34_POWER  0x4C
#define ES7210_ANALOG_CTRL  0x40

/* ─── ES8311 Register Definitions ─── */
#define ES8311_REG00_RESET          0x00
#define ES8311_REG01_CLK_MANAGER1   0x01
#define ES8311_REG02_CLK_MANAGER2   0x02
#define ES8311_REG03_CLK_MANAGER3   0x03
#define ES8311_REG04_CLK_MANAGER4   0x04
#define ES8311_REG05_CLK_MANAGER5   0x05
#define ES8311_REG06_CLK_MANAGER6   0x06
#define ES8311_REG07_CLK_MANAGER7   0x07
#define ES8311_REG08_CLK_MANAGER8   0x08
#define ES8311_REG09_SDP_IN         0x09
#define ES8311_REG0A_SDP_OUT        0x0A
#define ES8311_REG0B_SYSTEM         0x0B
#define ES8311_REG0C_SYSTEM         0x0C
#define ES8311_REG0D_SYSTEM         0x0D
#define ES8311_REG0E_SYSTEM         0x0E
#define ES8311_REG0F_ADC            0x0F
#define ES8311_REG10_ADC            0x10
#define ES8311_REG12_ADC_ALC        0x12
#define ES8311_REG17_ADC_VOLUME     0x17
#define ES8311_REG15_DAC            0x15
#define ES8311_REG32_DAC_VOLUME     0x32
#define ES8311_REG44_GPIO           0x44
#define ES8311_REGFD_CHD1           0xFD
#define ES8311_REGFE_CHD2           0xFE

/*
 * ────────────────────────────────────────────────
 *  I2C Helper Functions
 * ────────────────────────────────────────────────
 */
static esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_write_to_device(I2C_PORT, dev_addr, buf, 2,
                                      pdMS_TO_TICKS(100));
}

static esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *val)
{
    return i2c_master_write_read_device(I2C_PORT, dev_addr,
                                        &reg, 1, val, 1,
                                        pdMS_TO_TICKS(100));
}

/*
 * ────────────────────────────────────────────────
 *  I2C Bus Initialization
 * ────────────────────────────────────────────────
 */
static esp_err_t i2c_bus_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_CLK_SPEED,
    };
    esp_err_t ret = i2c_param_config(I2C_PORT, &conf);
    if (ret != ESP_OK) return ret;
    return i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
}

/*
 * ────────────────────────────────────────────────
 *  TCA9555 GPIO Expander: Enable PA (speaker amp)
 *  EXIO8 = bit 0 of Port 1 → must be driven HIGH
 * ────────────────────────────────────────────────
 */
static esp_err_t tca9555_enable_pa(void)
{
    // Read CONFIG1 (Port 1 config)
    uint8_t cfg1;
    if (i2c_read_reg(TCA9555_ADDR, TCA9555_CONFIG1, &cfg1) == ESP_OK) {
        cfg1 &= ~(1 << TCA9555_PA_EN_BIT); // Set EXIO8 as output
        i2c_write_reg(TCA9555_ADDR, TCA9555_CONFIG1, cfg1);
    }
    // Set EXIO8 HIGH
    uint8_t out1;
    if (i2c_read_reg(TCA9555_ADDR, TCA9555_OUTPUT1, &out1) == ESP_OK) {
        out1 |= (1 << TCA9555_PA_EN_BIT);
        i2c_write_reg(TCA9555_ADDR, TCA9555_OUTPUT1, out1);
        ESP_LOGI(TAG, "TCA9555: PA (speaker amplifier) ENABLED");
    }

    // Read CONFIG0 (Port 0 config)
    uint8_t cfg0;
    if (i2c_read_reg(TCA9555_ADDR, TCA9555_CONFIG0, &cfg0) == ESP_OK) {
        cfg0 &= ~(1 << 5); // Set EXIO5 (mic_en) as output
        i2c_write_reg(TCA9555_ADDR, TCA9555_CONFIG0, cfg0);
    }
    // Set EXIO5 HIGH
    uint8_t out0;
    if (i2c_read_reg(TCA9555_ADDR, TCA9555_OUTPUT0, &out0) == ESP_OK) {
        out0 |= (1 << 5); // HIGH = Mic Enabled
        i2c_write_reg(TCA9555_ADDR, TCA9555_OUTPUT0, out0);
        ESP_LOGI(TAG, "TCA9555: MIC ENABLED");
    }
    return ESP_OK;
}

/*
 * ────────────────────────────────────────────────
 *  ES7210 ADC (Microphone) Initialization
 * ────────────────────────────────────────────────
 */
static esp_err_t es7210_init(void)
{
    uint8_t val = 0;
    esp_err_t ret = i2c_read_reg(ES7210_ADDR, 0x00, &val);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ES7210 NOT FOUND at 0x%02X!", ES7210_ADDR);
        return ret;
    }
    ESP_LOGI(TAG, "ES7210 detected at 0x%02X", ES7210_ADDR);

    /* Software reset */
    i2c_write_reg(ES7210_ADDR, 0x00, 0xFF);
    i2c_write_reg(ES7210_ADDR, 0x00, 0x32);
    i2c_write_reg(ES7210_ADDR, 0x01, 0x3F);

    /* Initialization time when device powers up */
    i2c_write_reg(ES7210_ADDR, 0x09, 0x30);
    i2c_write_reg(ES7210_ADDR, 0x0A, 0x30);

    /* Configure HPF */
    i2c_write_reg(ES7210_ADDR, 0x23, 0x2A);
    i2c_write_reg(ES7210_ADDR, 0x22, 0x0A);
    i2c_write_reg(ES7210_ADDR, 0x20, 0x0A);
    i2c_write_reg(ES7210_ADDR, 0x21, 0x2A);

    /* Slave mode */
    i2c_write_reg(ES7210_ADDR, 0x08, 0x00);

    /* Analog power and mic bias */
    i2c_write_reg(ES7210_ADDR, 0x40, 0xC3);
    i2c_write_reg(ES7210_ADDR, 0x41, 0x70);
    i2c_write_reg(ES7210_ADDR, 0x42, 0x70);

    /* I2S Format: 16-bit I2S Standard Philips Mode (0x00) */
    i2c_write_reg(ES7210_ADDR, 0x11, 0x00);

    /* Sample rate: 16kHz with 4.096MHz MCLK */
    /* adc_div=1, doubler=0, dll=1 -> 0x01 | 0x00 | 0x80 = 0x81 */
    i2c_write_reg(ES7210_ADDR, 0x02, 0x81);
    i2c_write_reg(ES7210_ADDR, 0x07, 0x20); // OSR
    i2c_write_reg(ES7210_ADDR, 0x04, 0x01); // LRCK_H
    i2c_write_reg(ES7210_ADDR, 0x05, 0x00); // LRCK_L

    /* Mic gains (+37.5dB max clean hardware capture) */
    i2c_write_reg(ES7210_ADDR, 0x4B, 0xFF);
    i2c_write_reg(ES7210_ADDR, 0x4C, 0xFF);
    i2c_write_reg(ES7210_ADDR, 0x01, 0x00); 
    i2c_write_reg(ES7210_ADDR, 0x4B, 0x00);
    i2c_write_reg(ES7210_ADDR, 0x4C, 0x00);

    i2c_write_reg(ES7210_ADDR, 0x43, 0x06); // +18dB clean Gain (prevents 32512 clipping)
    i2c_write_reg(ES7210_ADDR, 0x44, 0x06);
    i2c_write_reg(ES7210_ADDR, 0x45, 0x06);
    i2c_write_reg(ES7210_ADDR, 0x46, 0x06);

    /* Power on mics */
    i2c_write_reg(ES7210_ADDR, 0x47, 0x08);
    i2c_write_reg(ES7210_ADDR, 0x48, 0x08);
    i2c_write_reg(ES7210_ADDR, 0x49, 0x08);
    i2c_write_reg(ES7210_ADDR, 0x4A, 0x08);

    /* Power down DLL */
    i2c_write_reg(ES7210_ADDR, 0x06, 0x04);

    /* Power on bias, ADC, PGA */
    i2c_write_reg(ES7210_ADDR, 0x4B, 0x00);
    i2c_write_reg(ES7210_ADDR, 0x4C, 0x00);

    /* I2S Interface 2 (Routing) */
    i2c_write_reg(ES7210_ADDR, 0x12, 0x00);

    /* Turn on ALL clocks */
    i2c_write_reg(ES7210_ADDR, 0x01, 0x00);

    /* Enable device */
    i2c_write_reg(ES7210_ADDR, 0x00, 0x71);
    i2c_write_reg(ES7210_ADDR, 0x00, 0x41);

    ESP_LOGI(TAG, "ES7210 microphone ADC initialized with +35dB gain");
    return ESP_OK;
}

/*
 * ────────────────────────────────────────────────
 *  ES8311 DAC (Speaker) Initialization
 * ────────────────────────────────────────────────
 */
static esp_err_t es8311_init(void)
{
    uint8_t chip_id1, chip_id2;
    esp_err_t ret = i2c_read_reg(ES8311_ADDR, 0xFD, &chip_id1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ES8311 NOT FOUND at 0x%02X!", ES8311_ADDR);
        return ret;
    }
    i2c_read_reg(ES8311_ADDR, 0xFE, &chip_id2);
    ESP_LOGI(TAG, "ES8311 detected! Chip ID: 0x%02X 0x%02X", chip_id1, chip_id2);

    /* Reset */
    i2c_write_reg(ES8311_ADDR, 0x00, 0x1F);
    i2c_write_reg(ES8311_ADDR, 0x00, 0x00);

    /* Clock config (16kHz with 4.096MHz MCLK) */
    i2c_write_reg(ES8311_ADDR, 0x01, 0x3F);
    i2c_write_reg(ES8311_ADDR, 0x02, 0x00); // 0x00 is mult x1
    i2c_write_reg(ES8311_ADDR, 0x03, 0x10);
    i2c_write_reg(ES8311_ADDR, 0x04, 0x20);
    i2c_write_reg(ES8311_ADDR, 0x05, 0x00);
    i2c_write_reg(ES8311_ADDR, 0x06, 0x03);
    i2c_write_reg(ES8311_ADDR, 0x07, 0x00);
    i2c_write_reg(ES8311_ADDR, 0x08, 0xFF);

    /* Format (16-bit I2S) */
    i2c_write_reg(ES8311_ADDR, 0x09, 0x0C);
    i2c_write_reg(ES8311_ADDR, 0x0A, 0x0C);

    /* Mic config */
    i2c_write_reg(ES8311_ADDR, 0x14, 0x1A);
    i2c_write_reg(ES8311_ADDR, 0x16, 0x00); // 0dB

    /* Set volume to lower, smooth level (-10dB / 0xAC) */
    i2c_write_reg(ES8311_ADDR, 0x32, 0xAC);

    /* Power up analog circuitry */
    i2c_write_reg(ES8311_ADDR, 0x0D, 0x01);
    /* Enable analog PGA, enable ADC modulator */
    i2c_write_reg(ES8311_ADDR, 0x0E, 0x02);
    /* Power up DAC */
    i2c_write_reg(ES8311_ADDR, 0x12, 0x00);
    /* Enable line out mode for NS4150B PA amplifier */
    i2c_write_reg(ES8311_ADDR, 0x13, 0x00);
    /* ADC Equalizer bypass, cancel DC offset in digital domain */
    i2c_write_reg(ES8311_ADDR, 0x1C, 0x6A);
    /* Bypass DAC equalizer */
    i2c_write_reg(ES8311_ADDR, 0x37, 0x08);

    /* Power On */
    i2c_write_reg(ES8311_ADDR, 0x00, 0x80);

    ESP_LOGI(TAG, "ES8311 DAC (speaker) initialized with ESPHome profile");
    return ESP_OK;
}

/*
 * ────────────────────────────────────────────────
 *  I2S Initialization — Full Duplex
 *
 *  ESP32-S3 I2S: The ESP is I2S master, both codecs are slaves.
 *  We create a full-duplex channel pair on I2S_NUM_0.
 * ────────────────────────────────────────────────
 */
static esp_err_t i2s_init(void)
{
    /* Create full-duplex channel: TX + RX on the same controller */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 8;
    chan_cfg.dma_frame_num = 512; // 512 frames = 32ms robust buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));

    /* Standard I2S Philips mode config */
    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCLK_PIN,
            .bclk = I2S_BCLK_PIN,
            .ws   = I2S_LRCK_PIN,
            .dout = I2S_DOUT_PIN,
            .din  = I2S_DIN_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    /* Init both TX and RX with the same config */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));

    /* Enable channels immediately so MCLK is generated for the codecs */
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    ESP_LOGI(TAG, "I2S initialized: %d Hz, %d-bit, mono, full-duplex",
             SAMPLE_RATE, BITS_PER_SAMPLE);
    return ESP_OK;
}

static void play_start_chime(void) {
    int16_t chime[1600 * 2]; // 100ms 16kHz stereo chime
    for (int i = 0; i < 1600; i++) {
        int16_t val = (i % 20 < 10) ? 5000 : -5000;
        chime[i * 2] = val;
        chime[i * 2 + 1] = val;
    }
    size_t bw;
    i2s_channel_write(tx_chan, chime, sizeof(chime), &bw, pdMS_TO_TICKS(500));
}

/*
 * ────────────────────────────────────────────────
 *  Main Application
 * ────────────────────────────────────────────────
 */
void app_main(void)
{
    /* Initialize LEDs */
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_NUM,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    if (led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip) == ESP_OK) {
        set_led_color(0, 0, 50); // Blue when switch on
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "   Waveshare ESP32-S3 Audio Repeater v2.0     ");
    ESP_LOGI(TAG, "   Record 3s -> Playback -> Repeat            ");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "");

    /* ── Step 1: Allocate recording buffer in PSRAM ── */
    ESP_LOGI(TAG, "[1/5] Allocating %d bytes (%d sec @ %d Hz) in PSRAM...",
             RECORD_SIZE, RECORD_SECONDS, SAMPLE_RATE);

    uint8_t *audio_buffer = (uint8_t *)heap_caps_malloc(RECORD_SIZE, MALLOC_CAP_SPIRAM);
    if (audio_buffer == NULL) {
        ESP_LOGW(TAG, "  PSRAM alloc failed, trying internal RAM...");
        audio_buffer = (uint8_t *)malloc(RECORD_SIZE);
    }
    if (audio_buffer == NULL) {
        ESP_LOGE(TAG, "  FATAL: Cannot allocate audio buffer! Halting.");
        return;
    }
    ESP_LOGI(TAG, "  Audio buffer OK");

    /* ── Step 0: Initialize NVS and Wi-Fi ── */
    ESP_LOGI(TAG, "[0/5] Initializing Wi-Fi...");
    wifi_init_sta();

    /* ── Step 1: Initialize LED Strip ── */
    ESP_LOGI(TAG, "[2/5] I2C bus init (SDA=GPIO%d, SCL=GPIO%d)...",
             I2C_SDA_PIN, I2C_SCL_PIN);
    ESP_ERROR_CHECK(i2c_bus_init());

    /* Scan I2C bus */
    ESP_LOGI(TAG, "  Scanning I2C bus...");
    for (int addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            const char *name = "";
            if (addr == ES8311_ADDR)  name = " (ES8311 DAC)";
            if (addr == ES7210_ADDR)  name = " (ES7210 ADC)";
            if (addr == TCA9555_ADDR) name = " (TCA9555 GPIO)";
            ESP_LOGI(TAG, "  Found device at 0x%02X%s", addr, name);
        }
    }

    /* ── Step 3: Enable speaker amplifier via TCA9555 ── */
    ESP_LOGI(TAG, "[3/5] Enabling speaker amplifier (TCA9555)...");
    tca9555_enable_pa();

    /* ── Step 4: Initialize I2S (Provides MCLK for codecs) ── */
    ESP_LOGI(TAG, "[4/5] Initializing I2S...");
    i2s_init();

    /* ── Step 5: Initialize audio codecs ── */
    ESP_LOGI(TAG, "[5/5] Initializing audio codecs...");

    esp_err_t ret = es7210_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "  ES7210 init failed (0x%X), mic may not work", ret);
    }

    ret = es8311_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "  ES8311 init failed (0x%X), speaker won't work!", ret);
    }

    /* ── Main record/playback loop ── */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, " AUDIO REPEATER RUNNING (STEREO)");
    ESP_LOGI(TAG, "==========================================");

    while (1) {
        /* ────── PHASE 1: RECORDING ────── */
        set_led_color(50, 50, 0); // Yellow when listening
        play_start_chime();
        ESP_LOGI(TAG, "🔔 [BEEP] >> RECORDING OPEN! SPEAK NOW! (%d seconds)", RECORD_SECONDS);

        size_t total_recorded = 0;
        memset(audio_buffer, 0, RECORD_SIZE);

        int max_amp_l = 0;
        int max_amp_r = 0;
        uint32_t last_print = 0;

        /* Feed silence to TX while recording (keeps clock alive) */
        uint8_t silence[1024];
        memset(silence, 0, sizeof(silence));
        const size_t chunk_size = 1024;

        while (total_recorded < RECORD_SIZE) {
            size_t to_read = chunk_size;
            if (total_recorded + to_read > RECORD_SIZE) {
                to_read = RECORD_SIZE - total_recorded;
            }

            /* Write silence to TX to keep the I2S bus clocked */
            size_t bytes_written_dummy;
            i2s_channel_write(tx_chan, silence, sizeof(silence), &bytes_written_dummy, 0);

            size_t bytes_read = 0;
            ret = i2s_channel_read(rx_chan,
                                    audio_buffer + total_recorded,
                                    to_read, &bytes_read,
                                    pdMS_TO_TICKS(1000));
                                    
            if (ret == ESP_OK && bytes_read > 0) {
                /* Calculate max amplitude for debugging and balance stereo */
                int16_t *samples = (int16_t *)(audio_buffer + total_recorded);
                int num_samples = bytes_read / 4; /* 2 channels, 2 bytes per sample = 4 bytes/frame */
                for (int i = 0; i < num_samples; i++) {
                    /* Copy active MIC1 (Left) to MIC2 (Right) for balanced stereo */
                    samples[i * 2 + 1] = samples[i * 2];

                    int16_t val_L = samples[i * 2];
                    int16_t val_R = samples[i * 2 + 1];
                    if (val_L < 0) val_L = -val_L;
                    if (val_R < 0) val_R = -val_R;
                    if (val_L > max_amp_l) max_amp_l = val_L;
                    if (val_R > max_amp_r) max_amp_r = val_R;
                }

                total_recorded += bytes_read;

                uint32_t current_ms = (total_recorded * 1000) / (SAMPLE_RATE * BYTES_PER_SAMPLE * CHANNELS);
                if (current_ms - last_print >= 500) {
                    ESP_LOGI(TAG, "  Recording: %.1f / %d s (Max Amp L: %d, R: %d)", 
                             current_ms / 1000.0, RECORD_SECONDS, max_amp_l, max_amp_r);
                    last_print = current_ms;
                    max_amp_l = 0;
                    max_amp_r = 0;
                }
            } else if (ret != ESP_OK) {
                ESP_LOGW(TAG, "  i2s_channel_read err: 0x%X", ret);
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        ESP_LOGI(TAG, "  Recording complete! %d bytes", total_recorded);

        ai_spoke = false;
        /* ────── PHASE 1.5: TRANSCRIBE & AI REPEAT ────── */
        set_led_color(50, 0, 50); // Purple when transcribing
        ESP_LOGI(TAG, ">> [AI] Transcribing speech & generating Aoede Human Voice...");
        stt_transcribe_audio(audio_buffer, total_recorded, on_stt_result);

        /* Small pause */
        vTaskDelay(pdMS_TO_TICKS(200));

        if (!ai_spoke) {
            ESP_LOGW(TAG, ">> [AI] No speech detected or transcribed. Please speak clearly into the microphone.");
            set_led_color(30, 0, 0); // Dim Red for empty cycle
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        set_led_color(0, 0, 50); // Blue when idle

        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "  Cycle done. Next recording in 2s...");
        ESP_LOGI(TAG, "");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
