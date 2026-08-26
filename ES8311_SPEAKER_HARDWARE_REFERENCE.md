# ES8311 Low Power Mono Audio CODEC — Speaker & DAC Subsystem Reference Manual

> **Document Type:** Core Electronics & Firmware Engineering Datasheet Specification  
> **Target Device:** Everest Semiconductor ES8311 (QFN-20)  
> **Domain:** Embedded Audio Playback / ESP32-S3 Speaker & Headphone Integration  

---

## 1. Executive Technical Summary

The **ES8311** integrates a high-fidelity, ultra-low-power mono **Digital-to-Analog Converter (DAC)** and a flexible analog output stage with an on-chip **headphone driver** and **differential line driver**. For speaker and headphone applications, the ES8311 receives digital PCM audio samples from an $I^2S$ host (such as an **ESP32-S3**), performs oversampling, digital filtering, dynamic range control (DRC), soft volume ramping, and multi-bit delta-sigma conversion, delivering up to **110 dB SNR** audio directly to earphones or an external Class-D power amplifier.

### Key Architectural Specifications (DAC & Output)
* **DAC Resolution:** 24-bit Multi-bit Delta-Sigma.
* **Sampling Frequencies ($F_s$):** 8 kHz to 96 kHz.
  * Single Speed Mode (SS): $8\text{ kHz} \le F_s \le 48\text{ kHz}$
  * Double Speed Mode (DS): $64\text{ kHz} \le F_s \le 96\text{ kHz}$
* **Dynamic Range / SNR:** $110\text{ dB}$ (A-weighted @ $AVDD = 3.3\text{V}$, $48\text{ kHz}$, $256 \times F_s$).
* **Total Harmonic Distortion + Noise (THD+N):** $-80\text{ dB}$ (Typ), $-75\text{ dB}$ (Max).
* **Analog Output Full-Scale Level:** $\pm 0.9 \times \frac{V_{AVDD}}{3.3}\text{ Vrms}$ (Single-Ended) / $\approx 1.8 \times \frac{V_{AVDD}}{3.3}\text{ Vrms}$ (Differential).
* **Output Topologies:**
  * **Differential Line Out / Power Amp Pre-Driver** (`OUTP` + `OUTN`)
  * **Headphone / Earphone Driver** (Integrated low-impedance driver mode via `HPSW`)
* **Hardware Digital Signal Processing:**
  * Dynamic Range Compression (DRC) & Peak Limiting
  * Hardware Parametric Equalizer (DACEQ) with 30-bit precision biquad filter coefficients
  * Digital Volume Attenuation/Gain: $-95.5\text{ dB}$ to $+32\text{ dB}$ ($0.5\text{ dB}/\text{step}$)
  * Anti-Pop & Click Noise Soft Ramping ($0.25\text{ dB} / 4\ \text{LRCK}$ to $65536\ \text{LRCK}$)
  * Hardware Sidetone / Zero-Latency Direct Mic Loopback (`ADC2DAC`)

---

## 2. QFN-20 Pin Roles in Audio Playback & Speaker Drive

```
                      +-------------------+
             CCLK -- | 1               20 | -- CE
             MCLK -- | 2    ES8311     19 | -- CDATA
             PVDD -- | 3    (Top View) 18 | -- [Mic Input / DMIC]
             DVDD -- | 4    QFN-20     17 | -- [Mic Input]
             DGND -- | 5               16 | -- VMID (1uF to AGND)
                      +-------------------+
                        6  7  8  9 10 11 12 13 14 15
                        |  |  |  |  |  |  |  |  |  |
                 SCLK --+  |  |  |  |  |  |  |  |  +-- ADCVREF (1uF to AGND)
           [ASDOUT] -------+  |  |  |  |  |  |  +----- DACVREF (1uF to AGND)
                   LRCK ------+  |  |  |  |  +-------- OUTN (Speaker - / HP Out)
                        DSDIN ---+  |  |  +----------- OUTP (Speaker + / HP Out)
                              AGND -+  +-- AVDD (3.3V)
```

### Complete Playback Pin Table

| Pin # | Pin Name | I/O Type | Domain | Electrical Function & Description |
|:---:|:---|:---:|:---:|:---|
| **1** | `CCLK` | Input (Digital) | $PVDD$ | $I^2C$ Control Port Serial Clock ($f_{SCL} \le 400\text{ kHz}$). |
| **2** | `MCLK` | Input (Digital) | $PVDD$ | Master Audio Clock ($256 \times F_s$). Synchronizes DAC delta-sigma modulator. |
| **3** | `PVDD` | Power | Supply | Digital I/O Pad Ring Power Supply ($1.6\text{V} - 3.6\text{V}$). |
| **4** | `DVDD` | Power | Supply | Digital Core Logic Supply ($1.6\text{V} - 3.6\text{V}$). |
| **5** | `DGND` | Ground | Supply | Digital Ground. |
| **6** | `SCLK` | Input / I/O | $PVDD$ | $I^2S$ Serial Bit Clock (BCLK). In slave mode, receives clock from ESP32-S3. |
| **8** | `LRCK` | Input / I/O | $PVDD$ | $I^2S$ Word Select / Frame Clock ($= F_s$). Determines Left / Right audio frame. |
| **9** | `DSDIN` | Input (Digital) | $PVDD$ | **DAC Serial Audio Data Input:** Receives playback PCM stream from ESP32 $I^2S$ output pin. Sampled on rising edge of `SCLK`. |
| **10** | `AGND` | Ground | Supply | Clean Analog Ground reference. |
| **11** | `AVDD` | Power | Supply | Analog Circuit Power Supply ($1.7\text{V} - 3.6\text{V}$, typ $3.3\text{V}$). Directly powers the DAC core and output amplifiers. |
| **12** | `OUTP` | Output (Analog) | $AVDD$ | **Positive Analog Audio Output:** Positive leg of differential line/speaker output, or Left/Mono headphone driver. |
| **13** | `OUTN` | Output (Analog) | $AVDD$ | **Negative Analog Audio Output:** Inverting leg of differential output, providing $180^\circ$ phase-inverted signal for bridge-tied load (BTL) external power amplifiers. |
| **14** | `DACVREF`| Analog Ref | $AVDD$ | **DAC Reference Voltage Filter Node:** Connect $1\ \mu\text{F}$ bypass capacitor directly to `AGND`. Critical for 110 dB SNR performance. |
| **16** | `VMID` | Analog Ref | $AVDD$ | **Mid-Rail Bias Node ($AVDD / 2$):** Connect $1\ \mu\text{F}$ ceramic capacitor to `AGND`. Supplies common-mode DC bias for output stages. |
| **19** | `CDATA` | Open-Drain / I/O | $PVDD$ | $I^2C$ Control Port Data Line. Requires external $2.2\text{k}\Omega - 4.7\text{k}\Omega$ pull-up. |
| **20** | `CE` | Input (Digital) | $PVDD$ | $I^2C$ Address Select Bit 0 ($0 \rightarrow \text{Address } 0x18$, $1 \rightarrow \text{Address } 0x19$). |
| **EP (21)**| `PGND / Exposed Pad`| Ground | Supply | Center thermal/ground pad. Must be soldered directly to PCB ground plane. |
| *7, 17, 18*| `ASDOUT, MIC1N, MIC1P` | - | - | *Microphone pins (Leave open/NC or ground if building a playback-only speaker device).* |

---

## 3. Speaker & Output Hardware Topologies

The ES8311 analog output stage can be wired in three distinct configurations depending on whether you are driving an external power amplifier, headphones, or a single-ended line input.

### 3.1 Topology 1: Differential Speaker Output (Driving External Class-D Power Amplifier)
*Recommended for loudspeakers, smart speakers, SoundBoxes, and IoT voice annunciators.*

The ES8311 outputs clean differential analog signals (`OUTP` and `OUTN`), which connect to an external audio power amplifier (such as **MAX98357, NS4150, NS4168, TPA3116, PAM8403, or LM4871**).

```
   ES8311 CODEC                          External Class-D Power Amp                Loudspeaker
  ┌──────────────┐                        ┌────────────────────────┐              ┌───────────┐
  │              │                        │                        │              │           │
  │  Pin 12 OUTP ├────||───[ 1 uF ]──────►│ IN+                VO+ ├─────────────►│ SPK (+)   │
  │              │                        │                        │              │           │
  │  Pin 13 OUTN ├────||───[ 1 uF ]──────►│ IN-                VO- ├─────────────►│ SPK (-)   │
  │              │                        │                        │              │ (4Ω / 8Ω) │
  │  Pin 10 AGND ├───────────────────────►│ GND                    │              └───────────┘
  └──────────────┘                        └────────────────────────┘
```
* **Why Differential?**
  1. **Doubles the voltage swing:** $V_{peak-peak(diff)} = 2 \times V_{peak-peak(SE)}$, providing $4\times$ the power into the power amplifier input.
  2. **Rejects Common-Mode Noise:** Cancels GSM/WiFi RF bursts and switching power supply ripple.
  3. **No DC Current:** Series $1\ \mu\text{F}$ capacitors block CODEC $VMID$ ($1.65\text{V}$) DC offset from entering the amplifier.
* **Register Settings:**
  * Reg `0x13[4] (HPSW) = 0` (Configured for Line Out drive).
  * Reg `0x12[1] (PDN_DAC) = 0` (Power on DAC).
  * Reg `0x12[0] (ENREFR) = 1` (Enable DAC reference output circuit).

---

### 3.2 Topology 2: Headphone / Earphone Driver (Direct Low-Impedance Drive)
*Used for driving $16\Omega$ to $32\Omega$ headphones directly from the ES8311 without an external amplifier.*

```
   ES8311 CODEC                                3.5mm Audio Jack / Earphones
  ┌──────────────┐                                ┌─────────────────────────┐
  │              │                                │                         │
  │  Pin 12 OUTP ├──────||──────[ 100 uF - 220 uF ]─►│ Tip (Left / Mono)       │
  │              │   (DC Block)                   │                         │
  │  Pin 13 OUTN ├──────||──────[ 100 uF - 220 uF ]─►│ Ring (Right / Inverted) │
  │              │                                │                         │
  │  Pin 10 AGND ├───────────────────────────────►│ Sleeve (GND)            │
  └──────────────┘                                └─────────────────────────┘
```
* **Capacitor Sizing:** Because headphones have low impedance ($16\Omega - 32\Omega$), large electrolytic/tantalum DC-blocking capacitors ($100\ \mu\text{F}$ to $220\ \mu\text{F}$) are required to maintain bass response:
  $$f_c = \frac{1}{2 \pi R C} = \frac{1}{2 \pi \times 32\Omega \times 100\ \mu\text{F}} \approx 49.7\text{ Hz}$$
* **Register Settings:**
  * Reg `0x13[4] (HPSW) = 1` (Enables integrated high-current Headphone Driver stage).

---

### 3.3 Topology 3: Single-Ended Line Output (Aux Out / Line-In to PC/Mixer)
*Used for standard $10\text{ k}\Omega$ line-level audio outputs.*

```
   ES8311 CODEC                                Line Out Jack
  ┌──────────────┐                                ┌─────────────────────────┐
  │  Pin 12 OUTP ├──────||──────[ 1 uF ]─────────►│ Signal Line Out         │
  │  Pin 13 OUTN ├────── (Leave Unconnected / NC) │                         │
  │  Pin 10 AGND ├───────────────────────────────►│ Shield / GND            │
  └──────────────┘                                └─────────────────────────┘
```

---

## 4. Digital Playback Signal Processing Pipeline

The digital audio path inside the ES8311 transforms raw incoming PCM bits into analog sound through 7 precision hardware blocks:

```
 [ESP32-S3 I2S Out]
        │
        ▼
 [Pin 9: DSDIN] ──────► Receives 16/24/32-bit PCM audio frames over I2S
        │
        ▼
 [SDP Input Mux] ─────► Left/Right channel slot selection [Reg 0x09]
        │
        ▼
 [Hardware DACEQ] ────► 30-bit precision parametric biquad equalizer [Reg 0x37-0x43]
        │
        ▼
 [Digital Volume] ────► -95.5 dB to +32 dB in 0.5 dB steps [Reg 0x32]
        │
        ▼
 [DRC / Peak Limiter] ► Dynamic Range Compression prevents speaker clipping [Reg 0x34, 0x35]
        │
        ▼
 [Soft Ramping Engine]► Anti-pop slew-rate volume ramping [Reg 0x37]
        │
        ▼
 [Delta-Sigma DAC] ───► Multi-bit Delta-Sigma Modulator (64x - 508x Fs) [Reg 0x04]
        │
        ▼
 [Analog HP Driver] ──► Line Out / Headphone Driver Amplifier [Reg 0x12, 0x13]
        │
        ▼
 [Pins 12 & 13: OUTP/OUTN] ──► Differential audio out to Speaker Amp or Headphones
```

---

## 5. Exhaustive DAC & Speaker Register Map (0x00 – 0xFF)

Below is the complete register reference for controlling the DAC, digital audio filters, dynamic range compressor, volume, and analog output stages.

---

### Register `0x00` – Reset & State Control
* **Address:** `0x00` | **Default:** `0x1F` (`0001 1111b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `CSM_ON` | R/W | `0` | **Chip State Machine Control:** `0`: Power down, `1`: Power on. |
| **6** | `MSC` | R/W | `0` | **Master/Slave Port Select:** `0`: Slave Mode (Host ESP32 provides SCLK & LRCK), `1`: Master Mode. |
| **0** | `RST_DAC_DIG` | R/W | `1` | **DAC Digital Core Reset:**<br>`0`: Normal operational run mode<br>`1`: Hold DAC digital processing logic in reset state |

---

### Register `0x01` – Clock Manager: DAC Clock Enables
* **Address:** `0x01` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **5** | `MCLK_ON` | R/W | `0` | **MCLK Buffer Enable:** `0`: Gated off, `1`: MCLK enabled and active. |
| **4** | `BCLK_ON` | R/W | `0` | **SDP Bit Clock Enable:** `0`: BCLK off, `1`: BCLK running. |
| **2** | `CLKDAC_ON`| R/W | `0` | **DAC Digital Clock Gate:**<br>`0`: `clk_dac` gated off<br>`1`: `clk_dac` clock running |
| **0** | `ANACLKDAC_ON`| R/W | `0` | **DAC Analog Clock Gate:**<br>`0`: DAC analog modulator clock off<br>`1`: DAC analog modulator clock running |

---

### Register `0x04` – Clock Manager: DAC Over-Sampling Ratio (OSR)
* **Address:** `0x04` | **Default:** `0x10` (`0001 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6:0** | `DAC_OSR[6:0]`| R/W | `0010000`| **DAC Delta-Sigma Oversampling Ratio:**<br>`0`–`14`: Reserved / Do not use<br>`15`: $60 \times F_s$ (DACEQ not available)<br>`16`: **$64 \times F_s$** (Default, standard audio mode)<br>`17`: $68 \times F_s$<br>`32`: $128 \times F_s$<br>`64`: **$256 \times F_s$** (High oversampling quality mode)<br>`127`: $508 \times F_s$ |

---

### Register `0x05` – Clock Manager: DAC Clock Divider
* **Address:** `0x05` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **3:0** | `DIV_CLKDAC[3:0]` | R/W | `0000` | **DAC Main Clock Divider:**<br>$\text{dac\_mclk} = \frac{\text{dig\_mclk}}{\text{DIV\_CLKDAC} + 1}$ ($1\times$ to $16\times$ division) |

---

### Register `0x09` – Serial Data Port (SDP) Input / DAC Settings *(Core Playback Register)*
* **Address:** `0x09` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `SDP_IN_SEL` | R/W | `0` | **DAC Audio Channel Slot Multiplexer:**<br>`0`: **Route Left Channel data from I2S stream to DAC**<br>`1`: **Route Right Channel data from I2S stream to DAC** |
| **6** | `SDP_IN_MUTE`| R/W | `0` | **DAC Digital Serial Input Mute:**<br>`0`: Unmute (Normal playback)<br>`1`: Mute (Forces zeroes into DAC input pipeline) |
| **5** | `SDP_IN_LRP` | R/W | `0` | **I2S Polarity & DSP Phase Selection:**<br>• In $I^2S$ / Left-Justified modes:<br>&nbsp;&nbsp;`0`: Normal Left/Right Channel polarity (`LRCK` low = Left, `LRCK` high = Right)<br>&nbsp;&nbsp;`1`: Inverted Left/Right Channel polarity<br>• In DSP/PCM Mode:<br>&nbsp;&nbsp;`0`: MSB appears on 2nd BCLK rising edge after LRCK pulse<br>&nbsp;&nbsp;`1`: MSB appears on 1st BCLK rising edge concurrent with LRCK |
| **4:2** | `SDP_IN_WL[2:0]`| R/W | `000` | **DAC Serial Audio Word Length (Bit-Depth):**<br>`000`: **24-bit audio depth** (Default)<br>`001`: **20-bit**<br>`010`: **18-bit**<br>`011`: **16-bit audio depth**<br>`100`: **32-bit audio slot** |
| **1:0** | `SDP_IN_FMT[1:0]`| R/W | `00` | **DAC Serial Audio Protocol Format:**<br>`00`: **$I^2S$ Standard Protocol** (Default)<br>`01`: **Left-Justified (LJ) Protocol**<br>`10`: Reserved<br>`11`: **DSP / PCM Mode** |

---

### Register `0x0D` – System Analog & Power Control 1
* **Address:** `0x0D` | **Default:** `0xFC` (`1111 1100b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `PDN_ANA` | R/W | `1` | **Master Analog Subsystem Power-Down:** `0`: Enabled, `1`: Powered down. |
| **6** | `PDN_IBIASGEN` | R/W | `1` | **Master Current Bias Power-Down:** `0`: Enabled, `1`: Powered down. |
| **3** | `PDN_DACVREFGEN`| R/W | `1` | **DAC Voltage Reference Generator Power-Down:**<br>`0`: **Enable DAC Reference ($DACVREF$ buffer active)**<br>`1`: Power down DAC Reference |
| **2** | `PDN_VREF` | R/W | `1` | **Master Internal Reference:** `0`: Disabled, `1`: **Enabled**. |
| **1:0** | `VMIDSEL[1:0]` | R/W | `00` | **Analog Mid-Rail ($VMID$) Operation:**<br>`00`: Power down (High-Z)<br>`01`: Fast startup / Normal speed charge on $VMID$ cap<br>`10`: **Normal operational $VMID$ mode** ($50\text{ k}\Omega$ internal divider)<br>`11`: Ultra-fast startup pre-charge mode |

---

### Register `0x0F` – System Low-Power DAC Overrides
* **Address:** `0x0F` | **Default:** `0x00` (`0000 0000b`)

| Bit | Bit Name | Function (`0`: Normal High Performance, `1`: Low Power Mode) |
|:---:|:---|:---|
| **7** | `LPDAC` | Low power mode for DAC core |
| **2** | `LPDACVRP`| Low power mode for DAC positive reference buffer |

---

### Register `0x10` – System Bias & DAC Current Tuning
* **Address:** `0x10` | **Default:** `0x13` (`0001 0011b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **4** | `DAC_IBIAS_SW` | R/W | `1` | **DAC Current Bias Setting:**<br>`0`: Normal DAC bias current<br>`1`: **Higher DAC bias setting (Default, lowest THD+N distortion)** |

---

### Register `0x12` – System DAC Power & Reference Enable *(Critical Power Register)*
* **Address:** `0x12` | **Default:** `0x02` (`0000 0010b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **1** | `PDN_DAC` | R/W | `1` | **DAC Analog Subsystem Power-Down:**<br>`0`: **Enable DAC Analog Stage & Modulator**<br>`1`: Power down DAC analog stage |
| **0** | `ENREFR` | R/W | `0` | **DAC Internal Output Reference Circuit:**<br>`0`: Disable internal reference circuits for DAC output<br>`1`: **Enable reference circuits for DAC output** (Must be `1` for sound) |

---

### Register `0x13` – Output Driver Mode Selection (Headphone vs Line Out)
* **Address:** `0x13` | **Default:** `0x40` (`0100 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **4** | `HPSW` | R/W | `0` | **Analog Output Driver Power & Impedance Switch:**<br>`0`: **Line Out Mode (Default):** High-linearity driver for external power amps<br>`1`: **Headphone Driver Mode:** High-current output driver for low-impedance $16\Omega - 32\Omega$ headphones |

---

### Register `0x31` – DAC Digital Mute & Modulation Controls
* **Address:** `0x31` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `DAC_DSMMUTE_TO`| R/W | `0` | **DAC Delta-Sigma Modulator Mute Target:** `0`: Mute to 8, `1`: Mute to 7/9. |
| **6** | `DAC_DSMMUTE` | R/W | `0` | **DAC Delta-Sigma Modulator Hard Mute:** `0`: Unmute (Normal), `1`: Hard mute. |
| **5** | `DAC_DEMMUTE` | R/W | `0` | **DAC Dynamic Element Matching (DEM) Mute:** `0`: Unmute, `1`: Mute. |
| **4** | `DAC_INV` | R/W | `0` | **DAC Output Phase Inversion:**<br>`0`: Normal phase (Default)<br>`1`: Invert audio output phase by $180^\circ$ |
| **3** | `DAC_RAMCLR` | R/W | `0` | **DAC RAM Delay Buffer Clear:** `0`: Normal, `1`: Clear RAM delay lines. |
| **2** | `DAC_DSMDITH_OFF`| R/W | `0` | **DAC Dithering Control:** `0`: Dither active (Reduces harmonic artifacts), `1`: Dither off. |

---

### Register `0x32` – DAC Digital Volume Control *(Master Volume)*
* **Address:** `0x32` | **Default:** `0x00` (Note: $0xBF = 0\text{ dB}$)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:0** | `DAC_VOLUME[7:0]`| R/W | `0x00` | **DAC Digital Gain / Attenuation ($0.5\text{ dB}$ per LSB):**<br>`0x00`: **$-95.5\text{ dB}$** (Digital Silence / Soft Mute)<br>`0x01`: **$-95.0\text{ dB}$**<br>... ($+0.5\text{ dB}$ per step)<br>`0xBE`: **$-0.5\text{ dB}$**<br>`0xBF`: **$0.0\text{ dB}$ (Unity Gain - 100% Volume)**<br>`0xC0`: **$+0.5\text{ dB}$**<br>...<br>`0xFF`: **$+32.0\text{ dB}$** (Digital Boost)<br>*(Note: When DRC is enabled, this register sets the DRC Maximum Gain ceiling)* |

---

### Register `0x33` – DAC DC Offset Calibration
* **Address:** `0x33` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:0** | `DAC_OFFSET[7:0]`| R/W | `0x00` | **DAC Output DC Offset Trim:** Fine digital offset adjustment to eliminate any remaining DC voltage at the DAC reconstruction filter. |

---

### Register `0x34` – Dynamic Range Compression (DRC) Enable & Window Size
* **Address:** `0x34` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `DRC_EN` | R/W | `0` | **Dynamic Range Compression (DRC) Enable:**<br>`0`: DRC disabled (Linear output)<br>`1`: **DRC active** (Compresses loud transients to prevent speaker blowout and distortion) |
| **3:0** | `DRC_WINSIZE[3:0]`| R/W | `0000` | **DRC Integration Window Size (Attack/Release Time):**<br>`0000`: $0.25\text{ dB} / 2\ \text{LRCK}$<br>`0001`: $0.25\text{ dB} / 4\ \text{LRCK}$<br>...<br>`1111`: $0.25\text{ dB} / 65536\ \text{LRCK}$ |

---

### Register `0x35` – DRC Target Output Levels (Compression Thresholds)
* **Address:** `0x35` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Threshold Table ($0\text{ to }15$) |
|:---:|:---|:---:|:---:|:---|
| **7:4** | `DRC_MAXLEVEL[3:0]`| R/W | `0000` | **DRC Maximum Limiter Ceiling (Peak threshold):**<br>`0`: $-30.1\text{ dB}$ \| `1`: $-24.1\text{ dB}$ \| `2`: $-20.6\text{ dB}$ \| `3`: $-18.1\text{ dB}$<br>`4`: $-16.1\text{ dB}$ \| `5`: $-14.5\text{ dB}$ \| `6`: $-13.2\text{ dB}$ \| `7`: $-12.0\text{ dB}$<br>`8`: $-11.0\text{ dB}$ \| `9`: $-10.1\text{ dB}$ \| `10`: $-9.3\text{ dB}$ \| `11`: $-8.5\text{ dB}$<br>`12`: $-7.8\text{ dB}$ \| `13`: $-7.2\text{ dB}$ \| `14`: $-6.6\text{ dB}$ \| `15`: **$-6.0\text{ dB}$** |
| **3:0** | `DRC_MINLEVEL[3:0]`| R/W | `0000` | **DRC Minimum Level Target:**<br>Same scale as `DRC_MAXLEVEL` ($-30.1\text{ dB}$ to $-6.0\text{ dB}$). |

---

### Register `0x37` – Anti-Pop Soft Volume Ramping & DACEQ Bypass
* **Address:** `0x37` | **Default:** `0x08` (`0000 1000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:4** | `DAC_RAMPRATE[3:0]`| R/W | `0000` | **Anti-Pop Soft Slew Rate (Smooth volume transitions):**<br>`0000`: Disable soft ramping (Instant volume jump)<br>`0001`: $0.25\text{ dB} / 4\ \text{LRCK}$ periods<br>`0010`: $0.25\text{ dB} / 8\ \text{LRCK}$<br>`0011`: $0.25\text{ dB} / 16\ \text{LRCK}$<br>`0100`: $0.25\text{ dB} / 32\ \text{LRCK}$<br>`0101`: $0.25\text{ dB} / 64\ \text{LRCK}$<br>`0110`: $0.25\text{ dB} / 128\ \text{LRCK}$<br>`0111`: $0.25\text{ dB} / 256\ \text{LRCK}$<br>`1000`: **$0.25\text{ dB} / 512\ \text{LRCK}$ (Default - Smooth & natural)**<br>`1001`: $0.25\text{ dB} / 1024\ \text{LRCK}$<br>`1111`: $0.25\text{ dB} / 65536\ \text{LRCK}$ (Ultra-slow fade) |
| **3** | `DAC_EQBYPASS` | R/W | `0` | **DAC Hardware Equalizer (DACEQ) Bypass:**<br>`0`: **DACEQ Enabled** (Biquad filter active)<br>`1`: **DACEQ Bypassed** (Clean linear audio pass-through) |

---

### Registers `0x38` to `0x43` – DAC 30-Bit Parametric Equalizer (DACEQ) Coefficients
Implements a 2nd-order IIR biquad acoustic tuning filter:
$$H(z) = \frac{B_0 + B_1 z^{-1}}{1 - A_1 z^{-1}}$$
Each coefficient ($B_0, B_1, A_1$) is a **30-bit signed two's complement** value mapped across 4 contiguous registers:

| Registers | Coeff Name | Bit Breakdown | Precision |
|:---|:---:|:---|:---:|
| `0x38` (bits 5:0), `0x39` (7:0), `0x3A` (7:0), `0x3B` (7:0) | **$B_0$** | `DACEQ_B0[29:0]` | 30-bit signed |
| `0x3C` (bits 7:0), `0x3D` (7:0), `0x3E` (7:0), `0x3F` (7:0) | **$B_1$** | `DACEQ_B1[29:0]` | 30-bit signed |
| `0x40` (bits 7:0), `0x41` (7:0), `0x42` (7:0), `0x43` (7:0) | **$A_1$** | `DACEQ_A1[29:0]` | 30-bit signed |

---

### Register `0x44` – Hardware Sidetone Loopback (`ADC2DAC`)
* **Address:** `0x44` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `ADC2DAC_SEL` | R/W | `0` | **Zero-Latency Direct Hardware Sidetone Loopback:**<br>`0`: Disabled (DAC plays audio received from `DSDIN`)<br>`1`: **Direct ADC-to-DAC Loopback Active** (Captured microphone audio is immediately routed into the DAC/Speaker with zero software latency) |

---

## 6. ESP32-S3 Firmware Playback Bring-Up Sequence (C Code)

To initialize the ES8311 for high-fidelity speaker/headphone playback from an ESP32-S3 over $I^2S$ ($Fs = 48\text{ kHz}$, 24-bit audio):

```c
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#define ES8311_I2C_ADDR 0x18 // 0x18 if Pin 20 (CE) = GND, 0x19 if CE = 3.3V

static esp_err_t es8311_write_reg(uint8_t reg, uint8_t val) {
    uint8_t data[2] = { reg, val };
    return i2c_master_write_to_device(I2C_NUM_0, ES8311_I2C_ADDR, data, 2, pdMS_TO_TICKS(100));
}

void es8311_speaker_init(void) {
    ESP_LOGI("ES8311", "Initializing Speaker & DAC Subsystem...");

    // 1. Soft Reset chip to clear any uninitialized states
    es8311_write_reg(0xFA, 0x01);
    vTaskDelay(pdMS_TO_TICKS(10));
    es8311_write_reg(0xFA, 0x00);

    // 2. Power on Internal Reference & Pre-charge VMID
    es8311_write_reg(0x0D, 0x01); // Fast VMID pre-charge
    es8311_write_reg(0x0E, 0x02); // Low-impedance reference buffer
    vTaskDelay(pdMS_TO_TICKS(25)); // Wait for VMID & DACVREF capacitors to charge
    es8311_write_reg(0x0D, 0x02); // Switch VMID to normal operating mode (AVDD/2)

    // 3. Configure Clock Management (Slave Mode, MCLK from ESP32)
    es8311_write_reg(0x00, 0x80); // CSM_ON = 1, Slave Mode (MSC = 0), un-reset blocks
    es8311_write_reg(0x01, 0x35); // MCLK_ON=1, BCLK_ON=1, CLKDAC_ON=1, ANACLKDAC_ON=1
    es8311_write_reg(0x02, 0x00); // MCLK pre-divider = 1, mult = 1
    es8311_write_reg(0x04, 0x10); // DAC OSR = 64 * Fs (or 0x40 for 256 * Fs)
    es8311_write_reg(0x05, 0x00); // DAC clock divider = 1

    // 4. Configure Serial Data Port Input (DSDIN playback stream from ESP32)
    // SDP_IN_SEL = 0 (Left channel to DAC), SDP_IN_WL = 000 (24-bit), SDP_IN_FMT = 00 (I2S)
    es8311_write_reg(0x09, 0x00);

    // 5. Configure Anti-Pop Soft Ramping & Equalizer Bypass
    es8311_write_reg(0x37, 0x88); // 0.25dB / 512 LRCK slew rate, DACEQ bypassed (0x08 | 0x80)

    // 6. Set Digital Volume to 0 dB Unity Gain (100% Volume)
    es8311_write_reg(0x32, 0xBF); // 0xBF = 0.0 dB, 0x00 = Mute, 0xFF = +32 dB

    // 7. Configure Output Stage & Driver Mode
    es8311_write_reg(0x13, 0x00); // HPSW = 0 (Line Out / External Speaker Amp mode)
                                  // Use 0x10 if driving low-impedance headphones directly

    // 8. Power On DAC Analog Core & Reference Output
    es8311_write_reg(0x12, 0x00); // PDN_DAC = 0 (DAC ON), ENREFR = 1 (Ref circuits enabled)

    // 9. Start Digital State Machine
    es8311_write_reg(0x00, 0x80); // Ensure State Machine is active and running

    ESP_LOGI("ES8311", "Speaker & DAC Subsystem successfully initialized and streaming!");
}
```

---

## 7. Speaker Engineer's Summary Checklist

| Subsystem | Requirement | Design Check |
|:---|:---|:---|
| **`DACVREF` Decoupling** | $1\ \mu\text{F}$ X7R ceramic capacitor on Pin 14 | Connect directly adjacent to Pin 14; return to `AGND`. |
| **`VMID` Decoupling** | $1\ \mu\text{F}$ X7R ceramic capacitor on Pin 16 | Maintains $AVDD/2$ DC bias; prevents pop noise during startup. |
| **Differential Output** | AC-couple `OUTP` and `OUTN` via $1\ \mu\text{F}$ caps | Connect to Differential inputs (`IN+`, `IN-`) of Class-D power amp. |
| **Headphone Mode** | Large DC-blocking caps ($100\ \mu\text{F} - 220\ \mu\text{F}$) | Required if driving $16\Omega - 32\Omega$ headphones directly. |
| **I2C Address Pin** | Tie `CE` (Pin 20) cleanly to `GND` or `PVDD` | `0x18` if low, `0x19` if high. Never leave `CE` floating. |
| **Playback-Only Devices**| Floating `ASDOUT` and tied `MIC1P`/`MIC1N` | Leave Pin 7 open; tie Pins 17 & 18 to `AGND` through caps to save power. |
