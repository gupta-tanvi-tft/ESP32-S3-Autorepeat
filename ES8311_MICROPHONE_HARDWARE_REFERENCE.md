# ES8311 Low Power Mono Audio CODEC — Hardware & Microphone Subsystem Reference Manual

> **Document Type:** Core Electronics & Firmware Engineering Datasheet Specification  
> **Target Device:** Everest Semiconductor ES8311 (QFN-20)  
> **Domain:** Embedded Audio Processing / ESP32-S3 Acoustic Front-End Integration  

---

## 1. Executive Technical Summary

The **ES8311** is a high-performance, ultra-low-power mono audio CODEC with an integrated multi-bit Delta-Sigma Analog-to-Digital Converter (ADC) and Digital-to-Analog Converter (DAC). For microphone input and audio forwarding applications, the device acts as an intelligent audio front-end (AFE) supporting both **analog microphones** (single-ended or differential) and **digital PDM/DMIC microphones**, offering on-chip Programmable Gain Amplification (PGA), Auto Level Control (ALC), Noise Gating, Auto-Mute, 2-stage High-Pass Filtering (HPF), and a 5-band/Biquad digital Parametric Equalizer (ADCEQ).

### Key Architectural Specifications
* **ADC Resolution:** 24-bit Multi-bit Delta-Sigma.
* **Sampling Frequencies ($F_s$):** 8 kHz to 96 kHz.
  * Single Speed Mode (SS): $8\text{ kHz} \le F_s \le 48\text{ kHz}$
  * Double Speed Mode (DS): $64\text{ kHz} \le F_s \le 96\text{ kHz}$
* **Dynamic Range / SNR:** $100\text{ dB}$ (A-weighted @ $3.3\text{V}$, $48\text{ kHz}$, $256\times F_s$).
* **Total Harmonic Distortion + Noise (THD+N):** $-93\text{ dB}$ (Typ), $-85\text{ dB}$ (Max).
* **Analog Input Full-Scale:** $\pm \frac{V_{AVDD}}{3.3}\text{ Vrms}$ (Differential / Single-ended).
* **Input Impedance:** $6\text{ k}\Omega$ (Typ).
* **Power Consumption:** $\approx 14\text{ mW}$ active record/playback; $< 1\ \mu\text{A}$ power-down standby.
* **Supply Voltage Ranges:**
  * Analog ($AVDD$): $1.7\text{V}$ to $3.6\text{V}$ (Typ $1.8\text{V}$ or $3.3\text{V}$)
  * Digital Core ($DVDD$): $1.6\text{V}$ to $3.6\text{V}$ (Typ $1.8\text{V}$ or $3.3\text{V}$)
  * I/O Pad Ring ($PVDD$): $1.6\text{V}$ to $3.6\text{V}$ (Typ $1.8\text{V}$ or $3.3\text{V}$)

---

## 2. Complete QFN-20 Pinout & Pin Role Classification

The ES8311 is packaged in a 20-pin QFN ($3\text{mm} \times 3\text{mm}$, $0.4\text{mm}$ pitch) with an exposed ground pad (EP / Pin 21).

```
                      +-------------------+
             CCLK -- | 1               20 | -- CE
             MCLK -- | 2    ES8311     19 | -- CDATA
             PVDD -- | 3    (Top View) 18 | -- MIC1P / DMIC_SDA
             DVDD -- | 4    QFN-20     17 | -- MIC1N
             DGND -- | 5               16 | -- VMID
                      +-------------------+
                        6  7  8  9 10 11 12 13 14 15
                        |  |  |  |  |  |  |  |  |  |
            SCLK/DMIC_SCL  |  |  |  |  |  |  |  |  ADCVREF
                    ASDOUT  |  |  |  |  |  |  DACVREF
                         LRCK  |  |  |  |  OUTN
                            DSDIN |  |  OUTP
                                AGND AVDD
```

### Complete 20-Pin Pinout Table

| Pin # | Pin Name | I/O Type | Domain | Electrical Function & Description |
|:---:|:---|:---:|:---:|:---|
| **1** | `CCLK` | Input (Digital) | $PVDD$ | $I^2C$ Control Port Serial Clock ($f_{SCL} \le 400\text{ kHz}$). |
| **2** | `MCLK` | Input (Digital) | $PVDD$ | Master Clock Input ($64F_s, 128F_s, 256F_s, 384F_s, 512F_s$, $12/24\text{ MHz}$, $16/25/26\text{ MHz}$). |
| **3** | `PVDD` | Power | Supply | Digital I/O Pad Ring Power Supply ($1.6\text{V} - 3.6\text{V}$). |
| **4** | `DVDD` | Power | Supply | Digital Core Logic Supply ($1.6\text{V} - 3.6\text{V}$). |
| **5** | `DGND` | Ground | Supply | Digital Ground. |
| **6** | `SCLK / DMIC_SCL` | I/O (Digital) | $PVDD$ | **Dual function:** $I^2S$ Serial Bit Clock (BCLK) in audio slave/master mode **OR** Digital PDM Microphone Clock Output (`DMIC_SCL`). |
| **7** | `ASDOUT` | Output (Digital) | $PVDD$ | ADC Serial Audio Data Output (Transmits captured microphone PCM audio to ESP32 / Host). |
| **8** | `LRCK` | I/O (Digital) | $PVDD$ | $I^2S$ Frame Clock / Word Select (Left/Right clock = $F_s$). |
| **9** | `DSDIN` | Input (Digital) | $PVDD$ | DAC Serial Audio Data Input (Playback data from Host). |
| **10** | `AGND` | Ground | Supply | Analog Ground Reference. |
| **11** | `AVDD` | Power | Supply | Analog Circuit Power Supply ($1.7\text{V} - 3.6\text{V}$). |
| **12** | `OUTP` | Output (Analog) | $AVDD$ | Differential Headphone / Line Output Positive terminal. |
| **13** | `OUTN` | Output (Analog) | $AVDD$ | Differential Headphone / Line Output Negative terminal. |
| **14** | `DACVREF`| Analog Ref | $AVDD$ | DAC Internal Voltage Reference Filter Node (Connect $1\ \mu\text{F}$ bypass cap to `AGND`). |
| **15** | `ADCVREF`| Analog Ref | $AVDD$ | ADC Internal Voltage Reference Filter Node (Connect $1\ \mu\text{F}$ bypass cap to `AGND`). |
| **16** | `VMID` | Analog Ref | $AVDD$ | Mid-Rail Bias Decoupling Point ($\approx \frac{AVDD}{2}$). Connect $1\ \mu\text{F}$ (or $10\ \mu\text{F} \| 0.1\ \mu\text{F}$) to `AGND`. |
| **17** | `MIC1N` | Input (Analog) | $AVDD$ | Negative Analog Microphone Input (Differential pair inverting input). |
| **18** | `MIC1P / DMIC_SDA` | Input (Analog / Dig) | $AVDD / PVDD$ | **Dual function:** Positive Analog Mic Input (`MIC1P`) **OR** Digital PDM Mic Data Input (`DMIC_SDA`). |
| **19** | `CDATA` | Open-Drain / I/O | $PVDD$ | $I^2C$ Control Port Serial Data line. Requires external pull-up ($2.2\text{k}\Omega - 4.7\text{k}\Omega$). |
| **20** | `CE` | Input (Digital) | $PVDD$ | $I^2C$ Device Address Select Bit 0 ($0 \rightarrow \text{Address } 0x18$, $1 \rightarrow \text{Address } 0x19$). |
| **EP (21)** | `PGND / Exposed Pad` | Ground | Supply | Exposed bottom thermal & electrical ground pad. Solder directly to PCB Ground plane. |

---

## 3. Microphone Input Topologies & Pin Connection Rules

The ES8311 provides flexibility in analog and digital acoustic capture. Below are the hardware configurations for each mode:

### 3.1 Mode A: Differential Analog Microphone (Recommended for Noise Immunity)
Used for ECM capsules with differential buffer or MEMS analog microphones with differential outputs ($OUT+, OUT-$).

* **Pin 18 (`MIC1P`)**: Connect to Microphone Output Positive via $1\ \mu\text{F}$ DC-blocking film/ceramic capacitor.
* **Pin 17 (`MIC1N`)**: Connect to Microphone Output Negative via $1\ \mu\text{F}$ DC-blocking film/ceramic capacitor.
* **Pin 16 (`VMID`)**: Decoupled with $1\ \mu\text{F} \| 0.1\ \mu\text{F}$ to `AGND`. Can act as reference for external bias divider.
* **Register Settings**:
  * Reg `0x14[6] (DMIC_ON) = 0` (Analog mode selected).
  * Reg `0x14[4] (LINSEL) = 1` (Selects `MIC1P - MIC1N` differential pair).
  * Reg `0x0E[6] (PDN_PGA) = 0` (Power on Analog Pre-Amp).
  * Reg `0x0E[5] (PDN_MOD) = 0` (Power on ADC Modulator).

```
   [Analog Diff Mic]
       (+) ---||---[1uF]---------> Pin 18 (MIC1P)
       (-) ---||---[1uF]---------> Pin 17 (MIC1N)
       GND ----------------------> AGND
```

### 3.2 Mode B: Single-Ended Analog Microphone
Used for standard single-ended ECM electret capsules or single-ended analog MEMS microphones.

* **Pin 18 (`MIC1P`)**: Connected to analog mic output via $1\ \mu\text{F}$ capacitor.
* **Pin 17 (`MIC1N`)**: AC-grounded via a matching $1\ \mu\text{F}$ capacitor to `AGND` (balances input impedance and common-mode rejection).
* **Bias Circuitry (for ECM)**: Microphone JFET powered from clean bias supply ($MICBIAS$ or filtered $AVDD$) through $2.2\text{ k}\Omega$ resistor.
* **Register Settings**:
  * Reg `0x14[6] (DMIC_ON) = 0`.
  * Reg `0x14[4] (LINSEL) = 1` (differential amplifier configured with inverting node AC-grounded).
  * Reg `0x0E[6] (PDN_PGA) = 0`.

```
   [Single-Ended Mic]
       Signal ---||---[1uF]------> Pin 18 (MIC1P)
       AGND -----||---[1uF]------> Pin 17 (MIC1N)
```

### 3.3 Mode C: Digital PDM Microphone (DMIC Mode)
Used with modern digital MEMS microphones emitting 1-bit PDM pulse streams.

* **Pin 6 (`SCLK / DMIC_SCL`)**: Outputs the PDM bit clock generated by the CODEC. Connected to Digital Mic `CLK`.
* **Pin 18 (`MIC1P / DMIC_SDA`)**: Switched to digital CMOS input mode to receive the 1-bit PDM bitstream from Digital Mic `DATA`.
* **Pin 17 (`MIC1N`)**: **UNUSED.** Leave floating or tie to `AGND` through a high impedance / capacitor.
* **Pin 15/16 (`ADCVREF`, `VMID`)**: Analog caps still populated for reference stability.
* **Register Settings**:
  * Reg `0x14[6] (DMIC_ON) = 1` (Enables DMIC mode, routing Pin 18 as `DMIC_SDA`).
  * Reg `0x15[0] (DMIC_SENSE) = 0` (Latch DMIC data on rising edge) or `1` (falling edge).
  * Reg `0x0E[6] (PDN_PGA) = 1` (Power DOWN analog PGA to save current).
  * Reg `0x0E[5] (PDN_MOD) = 1` (Power DOWN analog modulator).

```
   [Digital PDM Mic]
       CLK  <--------------------- Pin 6 (DMIC_SCL / SCLK)
       DATA ---------------------> Pin 18 (DMIC_SDA / MIC1P)
       VDD  <--------------------- PVDD / 3.3V
       GND  ---------------------> DGND
       Pin 17 (MIC1N) -----------> UNUSED (NC / AC-ground)
```

---

## 4. Audio Forwarding & Digital Interface Subsystem

The captured acoustic signal is processed through the digital core and forwarded across the **Serial Data Port (SDP)** to the Host MCU (e.g., ESP32-S3).

### 4.1 Digital Audio Signal Flow Diagram

```
 [Acoustic Sound]
       │
       ▼
 [MIC Input: Pin 18/17]
       │
       ▼
 [Analog PGA] ────────► 0 dB to +30 dB (3 dB steps) [Reg 0x14]
       │
       ▼
 [Delta-Sigma Modulator] Multi-bit Over-sampled Modulator
       │
       ▼
 [Decimation Filter] ──► Downsampled to Fs (8 kHz - 96 kHz)
       │
       ▼
 [Digital HPF 1 & 2] ──► DC Offset removal & low frequency cutoff [Reg 0x1B, 0x1C]
       │
       ▼
 [ADCEQ Equalizer] ────► 5-band / Biquad filter (30-bit precision coeffs) [Reg 0x1D-0x30]
       │
       ▼
 [ALC / Noise Gate] ───► Dynamic range auto-leveling & automute [Reg 0x18, 0x19, 0x1A]
       │
       ▼
 [ADC Gain Scaler] ────► 0 dB to +42 dB (6 dB steps) [Reg 0x16]
       │
       ▼
 [Digital Volume] ─────► -95.5 dB to +32 dB (0.5 dB steps) [Reg 0x17]
       │
       ▼
 [SDP Multiplexer] ────► Format selection: I2S / Left-Justified / DSP-PCM [Reg 0x0A, 0x44]
       │
       ▼
 [Pin 7: ASDOUT] ──────► Transmitted to ESP32-S3 I2S_IN (DMA buffer)
```

### 4.2 Serial Data Port (SDP) Formats
Forwarding supports 4 standard formats configurable via Register `0x0A[1:0]`:
1. **$I^2S$ Standard (Default):** MSB is available on the 2nd BCLK rising edge after LRCK edge transition. LRCK low = Left channel, LRCK high = Right channel.
2. **Left-Justified:** MSB is available immediately on the 1st BCLK rising edge concurrent with LRCK transition.
3. **DSP/PCM Mode A:** MSB is available on the 2nd BCLK rising edge after LRCK pulsed high (1 BCLK pulse width).
4. **DSP/PCM Mode B:** MSB is available on the 1st BCLK rising edge after LRCK pulse.

### 4.3 Word Lengths Supported
Configurable via Register `0x0A[4:2]`:
* `000`: **24-bit** (Default)
* `001`: **20-bit**
* `010`: **18-bit**
* `011`: **16-bit**
* `100`: **32-bit**

### 4.4 Master vs. Slave Modes
* **Slave Mode (Default, Reg `0x00[6] = 0`):** ESP32-S3 supplies `MCLK`, `SCLK (BCLK)`, and `LRCK (WS)`. ES8311 synchronizes internal counters and pushes data to `ASDOUT` on the falling edge of `SCLK`.
* **Master Mode (Reg `0x00[6] = 1`):** ES8311 divides internal `MCLK` using internal dividers (Reg `0x06`, `0x07`, `0x08`) and drives `SCLK` and `LRCK` outwards to the host.

---

## 5. Exhaustive Register Map & Bitfield Analysis (0x00 – 0xFF)

Below is the complete, bit-by-bit register directory of the ES8311 CODEC.

---

### Register `0x00` – Reset & State Control
* **Address:** `0x00` | **Default:** `0x1F` (`0001 1111b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `CSM_ON` | R/W | `0` | **Chip State Machine Control:**<br>`0`: CSM power down / IDLE<br>`1`: CSM power on / operational |
| **6** | `MSC` | R/W | `0` | **Master/Slave Serial Port Selection:**<br>`0`: Slave serial port mode (Host supplies SCLK & LRCK)<br>`1`: Master serial port mode (ES8311 drives SCLK & LRCK) |
| **5** | `SEQ_DIS` | R/W | `0` | **Power-up Sequencing Control:**<br>`0`: Automatic internal power-up sequencing enabled<br>`1`: Power-up sequence disabled |
| **4** | `RST_DIG` | R/W | `1` | **Digital Core Reset:**<br>`0`: Normal operational mode<br>`1`: Reset digital core except I2C control registers |
| **3** | `RST_CMG` | R/W | `1` | **Clock Manager Block Reset:**<br>`0`: Normal<br>`1`: Clock manager held in reset |
| **2** | `RST_MST` | R/W | `1` | **Master Mode Clock Block Reset:**<br>`0`: Normal<br>`1`: Reset master clock generation block |
| **1** | `RST_ADC_DIG` | R/W | `1` | **ADC Digital Filter Reset:**<br>`0`: Normal operation<br>`1`: ADC digital filter logic held in reset |
| **0** | `RST_DAC_DIG` | R/W | `1` | **DAC Digital Filter Reset:**<br>`0`: Normal operation<br>`1`: DAC digital filter logic held in reset |

---

### Register `0x01` – Clock Manager: Clock Enables & Multiplexers
* **Address:** `0x01` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `MCLK_SEL` | R/W | `0` | **Main Clock Input Routing:**<br>`0`: System clock sourced from `MCLK` pin (Pin 2)<br>`1`: System clock sourced from `BCLK` pin (Pin 6) |
| **6** | `MCLK_INV` | R/W | `0` | **MCLK Phase Inversion:**<br>`0`: Non-inverted MCLK<br>`1`: Invert MCLK input phase |
| **5** | `MCLK_ON` | R/W | `0` | **MCLK Buffer Enable:**<br>`0`: MCLK input gated/off<br>`1`: MCLK enabled and routed internally |
| **4** | `BCLK_ON` | R/W | `0` | **SDP Bit Clock Enable:**<br>`0`: BCLK off<br>`1`: BCLK on |
| **3** | `CLKADC_ON`| R/W | `0` | **ADC Digital Clock Gate:**<br>`0`: `clk_adc` gated off<br>`1`: `clk_adc` clock running |
| **2** | `CLKDAC_ON`| R/W | `0` | **DAC Digital Clock Gate:**<br>`0`: `clk_dac` gated off<br>`1`: `clk_dac` clock running |
| **1** | `ANACLKADC_ON` | R/W | `0` | **ADC Analog Modulator Clock Gate:**<br>`0`: Modulator clock off<br>`1`: Modulator analog clock running |
| **0** | `ANACLKDAC_ON` | R/W | `0` | **DAC Analog Clock Gate:**<br>`0`: DAC analog clock off<br>`1`: DAC analog clock running |

---

### Register `0x02` – Clock Manager: Pre-dividers & Clock Multipliers
* **Address:** `0x02` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:5** | `DIV_PRE[2:0]` | R/W | `000` | **Pre-divider for MCLK in:**<br>$\text{mclk\_prediv} = \frac{MCLK}{\text{DIV\_PRE} + 1}$ ($1\times$ to $8\times$ division) |
| **4:3** | `MULT_PRE[1:0]`| R/W | `00` | **Internal Frequency Multiplier (Clock Doubler/PLL):**<br>`00`: $\text{dig\_mclk} = \text{mclk\_prediv} \times 1$<br>`01`: $\text{dig\_mclk} = \text{mclk\_prediv} \times 2$<br>`10`: $\text{dig\_mclk} = \text{mclk\_prediv} \times 4$<br>`11`: $\text{dig\_mclk} = \text{mclk\_prediv} \times 8$<br>*(Note: When $\times 4$ or $\times 8$ is selected, $\text{mclk\_prediv} > 500\text{ kHz} @ 1.8\text{V}$, or $> 1\text{ MHz} @ 3.3\text{V}$)* |
| **2** | `PATHSEL` | R/W | `0` | **Clock Doubler Path Select:**<br>`0`: Direct non-DFF path<br>`1`: DFF path |
| **1:0** | `DELYSEL[1:0]` | R/W | `00` | **Clock Doubler Delay Cell Select:**<br>`00`: $5\text{ ns}$ delay<br>`01`: $10\text{ ns}$ delay<br>`10`: $15\text{ ns}$ delay<br>`11`: $15\text{ ns}$ delay |

---

### Register `0x03` – Clock Manager: ADC Over-Sampling Ratio (OSR) & Speed
* **Address:** `0x03` | **Default:** `0x10` (`0001 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6** | `ADC_FSMODE` | R/W | `0` | **ADC Sample Speed Mode:**<br>`0`: Single Speed Mode ($Fs = 8\text{ kHz} - 48\text{ kHz}$)<br>`1`: Double Speed Mode ($Fs = 64\text{ kHz} - 96\text{ kHz}$) |
| **5:0** | `ADC_OSR[5:0]`| R/W | `010000` | **ADC Delta-Sigma Oversampling Ratio Selection:**<br>`0` to `14`: Reserved / Do not use<br>`15`: $60 \times F_s$ (SS only)<br>`16`: $64 \times F_s$ (SS) / $32 \times F_s$ (DS) (Default)<br>`31`: $124 \times F_s$ (SS) / $62 \times F_s$ (DS)<br>`32`: $128 \times F_s$ (SS) / $64 \times F_s$ (DS)<br>`63`: $252 \times F_s$ (SS) / $126 \times F_s$ (DS) |

---

### Register `0x04` – Clock Manager: DAC Over-Sampling Ratio (OSR)
* **Address:** `0x04` | **Default:** `0x10` (`0001 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6:0** | `DAC_OSR[6:0]`| R/W | `0010000`| **DAC Oversampling Ratio:**<br>`0` to `14`: Reserved<br>`15`: $60 \times F_s$ (EQ disabled)<br>`16`: $64 \times F_s$ (Default)<br>`32`: $128 \times F_s$<br>`64`: $256 \times F_s$<br>`127`: $508 \times F_s$ |

---

### Register `0x05` – Clock Manager: ADC & DAC MCLK Dividers
* **Address:** `0x05` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:4** | `DIV_CLKADC[3:0]` | R/W | `0000` | **ADC Clock Divider:**<br>$\text{adc\_mclk} = \frac{\text{dig\_mclk}}{\text{DIV\_CLKADC} + 1}$ |
| **3:0** | `DIV_CLKDAC[3:0]` | R/W | `0000` | **DAC Clock Divider:**<br>$\text{dac\_mclk} = \frac{\text{dig\_mclk}}{\text{DIV\_CLKDAC} + 1}$ |

---

### Register `0x06` – Clock Manager: Master Mode BCLK Divider & Polarity
* **Address:** `0x06` | **Default:** `0x03` (`0000 0011b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6** | `BCLK_CON` | R/W | `0` | **Master BCLK Continuous Control:**<br>`0`: Continuous BCLK output<br>`1`: Gated BCLK (stops when data transfer is finished) |
| **5** | `BCLK_INV` | R/W | `0` | **BCLK Inversion:**<br>`0`: Normal polarity<br>`1`: Inverted BCLK polarity |
| **4:0** | `DIV_BCLK[4:0]` | R/W | `00011` | **Master Mode BCLK Frequency Divider:**<br>`0`–`19`: $\text{BCLK} = \frac{MCLK}{\text{DIV\_BCLK} + 1}$ (Default `3` $\rightarrow \frac{MCLK}{4}$)<br>`20`: $\frac{MCLK}{22}$ \| `21`: $\frac{MCLK}{24}$ \| `22`: $\frac{MCLK}{25}$ \| `23`: $\frac{MCLK}{30}$<br>`24`: $\frac{MCLK}{32}$ \| `25`: $\frac{MCLK}{33}$ \| `26`: $\frac{MCLK}{34}$ \| `27`: $\frac{MCLK}{36}$<br>`28`: $\frac{MCLK}{44}$ \| `29`: $\frac{MCLK}{48}$ \| `30`: $\frac{MCLK}{66}$ \| `31`: $\frac{MCLK}{72}$ |

---

### Register `0x07` – Clock Manager: Tri-State & Master LRCK Divider MSB
* **Address:** `0x07` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **5** | `TRI_BLRCK` | R/W | `0` | **BCLK / LRCK Tri-State Output:**<br>`0`: Normal active drive<br>`1`: High-impedance (Hi-Z) output |
| **4** | `TRI_ADCDAT` | R/W | `0` | **ASDOUT Tri-State Control:**<br>`0`: Normal active output<br>`1`: ASDOUT held in High-Z state |
| **3:0** | `DIV_LRCK[11:8]`| R/W | `0000` | **Master LRCK Divider Upper 4 bits** (Bits 11:8). |

---

### Register `0x08` – Clock Manager: Master LRCK Divider LSB
* **Address:** `0x08` | **Default:** `0xFF` (`1111 1111b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:0** | `DIV_LRCK[7:0]` | R/W | `11111111`| **Master LRCK Divider Lower 8 bits:**<br>$\text{LRCK (Master)} = \frac{MCLK}{\text{DIV\_LRCK}[11:0] + 1}$ |

---

### Register `0x09` – Serial Data Port (SDP) Input / DAC Settings
* **Address:** `0x09` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `SDP_IN_SEL` | R/W | `0` | **DAC Channel Slot Selection:**<br>`0`: Left channel data to DAC<br>`1`: Right channel data to DAC |
| **6** | `SDP_IN_MUTE`| R/W | `0` | **DAC Digital Input Mute:** `0`: Unmute, `1`: Mute |
| **5** | `SDP_IN_LRP` | R/W | `0` | **I2S Polarity / DSP Alignment:**<br>In I2S/LJ: `0`: Normal L/R polarity, `1`: Inverted L/R<br>In DSP/PCM: `0`: MSB on 2nd BCLK edge, `1`: MSB on 1st BCLK edge |
| **4:2** | `SDP_IN_WL[2:0]` | R/W | `000` | **DAC Data Word Length:**<br>`000`: 24-bit, `001`: 20-bit, `010`: 18-bit, `011`: 16-bit, `100`: 32-bit |
| **1:0** | `SDP_IN_FMT[1:0]`| R/W | `00` | **DAC Serial Audio Format:**<br>`00`: $I^2S$, `01`: Left-Justified, `10`: Reserved, `11`: DSP/PCM |

---

### Register `0x0A` – Serial Data Port (SDP) Output / ADC Settings *(Crucial for Mic Capture)*
* **Address:** `0x0A` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6** | `SDP_OUT_MUTE` | R/W | `0` | **ADC Output Serial Data Mute:**<br>`0`: Unmute (transmits audio)<br>`1`: Mute (outputs zero frames on `ASDOUT`) |
| **5** | `SDP_OUT_LRP` | R/W | `0` | **ADC Serial Data Polarity & DSP Phase Selection:**<br>• In $I^2S$ / Left-Justified mode:<br>&nbsp;&nbsp;`0`: Normal Left/Right Channel polarity<br>&nbsp;&nbsp;`1`: Inverted Left/Right Channel polarity<br>• In DSP/PCM Mode:<br>&nbsp;&nbsp;`0`: Mode A (MSB appears on 2nd BCLK rising edge after LRCK pulse)<br>&nbsp;&nbsp;`1`: Mode B (MSB appears on 1st BCLK rising edge concurrent with LRCK) |
| **4:2** | `SDP_OUT_WL[2:0]`| R/W | `000` | **ADC Serial Data Output Bit-Depth:**<br>`000`: **24-bit audio sample** (Default)<br>`001`: **20-bit**<br>`010`: **18-bit**<br>`011`: **16-bit audio sample**<br>`100`: **32-bit audio slot** |
| **1:0** | `SDP_OUT_FMT[1:0]`| R/W | `00` | **ADC Serial Audio Bus Protocol Format:**<br>`00`: **$I^2S$ Standard Protocol** (Default)<br>`01`: **Left-Justified (LJ) Protocol**<br>`10`: Reserved<br>`11`: **DSP / PCM Mode** |

---

### Register `0x0B` & `0x0C` – System Power-Up Timing Sequencing
* **Reg `0x0B`:** `0x00` | **Reg `0x0C`:** `0x20`

| Register | Bit(s) | Bit Name | Description / Timing |
|:---|:---:|:---|:---|
| **0x0B** | **7:3** | `PWRUP_A[4:0]` | **Power Up Stage A Timing Control:**<br>$0 - 31$: $21\ \mu\text{s} - 232\text{ ms}$ (@ $Fs = 48\text{ kHz}$)<br>$0 - 31$: $23\ \mu\text{s} - 253\text{ ms}$ (@ $Fs = 44.1\text{ kHz}$)<br>$0 - 31$: $120\ \mu\text{s} - 1392\text{ ms}$ (@ $Fs = 8\text{ kHz}$) |
| **0x0B** | **2:0** | `PWRUP_B[3:1]` | **Power Up Stage B Upper Bits** ($21\ \mu\text{s} - 104\text{ ms}$ @ $48\text{ kHz}$). |
| **0x0C** | **7** | `PWRUP_B[0]` | **Power Up Stage B Bit 0**. |
| **0x0C** | **6:0** | `PWRUP_C[6:0]` | **Power Up Stage C Timing Control:**<br>$0 - 31$: $21\ \mu\text{s} - 234\text{ ms}$ (@ $Fs = 48\text{ kHz}$). |

---

### Register `0x0D` – System Analog & Power-Down Control 1
* **Address:** `0x0D` | **Default:** `0xFC` (`1111 1100b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `PDN_ANA` | R/W | `1` | **Master Analog Subsystem Power-Down:**<br>`0`: Enable all analog circuitry<br>`1`: Power down master analog subsystem |
| **6** | `PDN_IBIASGEN` | R/W | `1` | **Analog Bias Generator Power-Down:**<br>`0`: Enable master current bias generator<br>`1`: Power down master current bias generator |
| **5** | `PDN_ADCBIASGEN`| R/W | `1` | **ADC Specific Bias Power-Down:**<br>`0`: Enable ADC analog bias circuits<br>`1`: Power down ADC analog bias circuits |
| **4** | `PDN_ADCVREFGEN`| R/W | `1` | **ADC Voltage Reference Generator Power-Down:**<br>`0`: Enable ADC Reference ($ADCVREF$ buffer active)<br>`1`: Power down ADC Reference generator |
| **3** | `PDN_DACVREFGEN`| R/W | `1` | **DAC Voltage Reference Generator Power-Down:**<br>`0`: Enable DAC Reference<br>`1`: Power down DAC Reference |
| **2** | `PDN_VREF` | R/W | `1` | **Internal Reference Master Power-Down:**<br>`0`: Reference circuits disabled<br>`1`: Internal Reference generator enabled (active) |
| **1:0** | `VMIDSEL[1:0]` | R/W | `00` | **Analog Mid-Rail ($VMID$) Voltage Mode & Charging Speed:**<br>`00`: $VMID$ powered down (High-Z)<br>`01`: Fast startup / Normal speed charge on $VMID$ cap<br>`10`: Normal operational $VMID$ mode ($50\text{ k}\Omega$ internal divider to $AVDD/2$)<br>`11`: Ultra-fast startup pre-charge mode |

---

### Register `0x0E` – System Analog & Power-Down Control 2 (PGA & Modulator)
* **Address:** `0x0E` | **Default:** `0x6A` (`0110 1010b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6** | `PDN_PGA` | R/W | `1` | **Analog Programmable Gain Pre-Amplifier (PGA) Power-Down:**<br>`0`: **PGA Enabled & active** (Used for analog mic)<br>`1`: **PGA Powered Down** (Set to `1` when using Digital Mic to save power) |
| **5** | `PDN_MOD` | R/W | `1` | **ADC Delta-Sigma Analog Modulator Power-Down:**<br>`0`: **ADC Modulator Enabled** (Active analog conversion)<br>`1`: Modulator powered down |
| **4** | `RST_MOD` | R/W | `0` | **ADC Modulator Reset:**<br>`0`: Modulator normal run<br>`1`: Hold ADC modulator in reset state |
| **3** | `VROI` | R/W | `1` | **Internal Reference Output Impedance:**<br>`0`: Normal output impedance<br>`1`: Low impedance reference mode |
| **2** | `LPVREFBUF` | R/W | `0` | **VREF Buffer Power Mode:**<br>`0`: Normal high-performance mode<br>`1`: Low power reference buffer mode |

---

### Register `0x0F` – System Low-Power Subsystem Overrides
* **Address:** `0x0F` | **Default:** `0x00` (`0000 0000b`)

| Bit | Bit Name | Function (`0`: Normal, `1`: Low Power Mode) |
|:---:|:---|:---|
| **7** | `LPDAC` | Low power mode for DAC |
| **6** | `LPPGA` | Low power mode for Analog PGA |
| **5** | `LPPGAOUT` | Low power mode for PGA output driver |
| **4** | `LPVCMMOD` | Low power mode for Common Mode Modulator buffer |
| **3** | `LPADCVRP` | Low power mode for ADC Positive Reference ($ADCVREF$) |
| **2** | `LPDACVRP` | Low power mode for DAC Positive Reference |
| **1** | `LPFLASH` | Low power mode for ADC Flash quantizer |
| **0** | `LPINT1` | Low power mode for ADC Integrator Stage 1 |

---

### Register `0x10` – System Bias & Reference Fine Tuning
* **Address:** `0x10` | **Default:** `0x13` (`0001 0011b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `SYNCMODE` | R/W | `0` | `0`: Normal mode, `1`: Synchronous mode |
| **6:5** | `VMIDLOW[1:0]` | R/W | `00` | **VMID Voltage Offset:**<br>`00`: $VMID = \frac{AVDD}{2}$<br>`01`: $VMID = \frac{AVDD}{2} - 75\text{ mV}$<br>`10`: $VMID = \frac{AVDD}{2} - 145\text{ mV}$<br>`11`: $VMID = \frac{AVDD}{2} - 175\text{ mV}$ |
| **4** | `DAC_IBIAS_SW` | R/W | `1` | `0`: Normal DAC bias, `1`: Higher DAC bias |
| **3:2** | `IBIAS_SW[1:0]` | R/W | `00` | **Master Analog Bias Tuning:**<br>`00`: Level 0 (Default)<br>`01`: Level 1<br>`10`: Level 2<br>`11`: Level 3 (Highest current bias / Lowest distortion) |
| **1** | `VX2OFF` | R/W | `1` | **Internal Voltage Doubler:** `0`: Doubler enabled, `1`: Doubler off |
| **0** | `VX1SEL` | R/W | `1` | **Internal Reference Target:** `0`: $1.45\text{V}$, `1`: $1.65\text{V}$ |

---

### Register `0x14` – System Signal Routing & Analog Microphone PGA Gain *(Core Mic Register)*
* **Address:** `0x14` | **Default:** `0x10` (`0001 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6** | `DMIC_ON` | R/W | `0` | **Digital Microphone Input Select:**<br>`0`: **Analog Microphone Mode** (`MIC1P`/`MIC1N` routed to PGA)<br>`1`: **Digital PDM Microphone Mode** (Pin 18 configured as `DMIC_SDA`, Pin 6 generates `DMIC_SCL`) |
| **4** | `LINSEL` | R/W | `1` | **Analog Input Differential Routing:**<br>`0`: No input selected (Analog frontend disconnected)<br>`1`: **Select `MIC1P - MIC1N` differential input** |
| **3:0** | `PGAGAIN[3:0]` | R/W | `0000` | **Analog Microphone Pre-Amplifier Gain (0 to +30 dB):**<br>`0000` (`0x0`): **0 dB**<br>`0001` (`0x1`): **+3 dB**<br>`0010` (`0x2`): **+6 dB**<br>`0011` (`0x3`): **+9 dB**<br>`0100` (`0x4`): **+12 dB**<br>`0101` (`0x5`): **+15 dB**<br>`0110` (`0x6`): **+18 dB**<br>`0111` (`0x7`): **+21 dB**<br>`1000` (`0x8`): **+24 dB**<br>`1001` (`0x9`): **+27 dB**<br>`1010` (`0xA`): **+30 dB**<br>`1011`–`1111`: Reserved |

---

### Register `0x15` – ADC Volume Soft Ramp & DMIC Latch Edge
* **Address:** `0x15` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:4** | `ADC_RAMPRATE[3:0]`| R/W | `0000` | **ADC Volume Control Soft Slew / Ramp Rate:**<br>`0000`: Disable soft ramping (Instant step changes)<br>`0001`: $0.25\text{ dB} / 4\ \text{LRCK}$ periods<br>`0010`: $0.25\text{ dB} / 8\ \text{LRCK}$<br>`0011`: $0.25\text{ dB} / 16\ \text{LRCK}$<br>`0100`: $0.25\text{ dB} / 32\ \text{LRCK}$<br>`0101`: $0.25\text{ dB} / 64\ \text{LRCK}$<br>`0110`: $0.25\text{ dB} / 128\ \text{LRCK}$<br>`0111`: $0.25\text{ dB} / 256\ \text{LRCK}$<br>`1000`: $0.25\text{ dB} / 512\ \text{LRCK}$<br>`1001`: $0.25\text{ dB} / 1024\ \text{LRCK}$<br>`1010`: $0.25\text{ dB} / 2048\ \text{LRCK}$<br>`1011`: $0.25\text{ dB} / 4096\ \text{LRCK}$<br>`1100`: $0.25\text{ dB} / 8192\ \text{LRCK}$<br>`1101`: $0.25\text{ dB} / 16384\ \text{LRCK}$<br>`1110`: $0.25\text{ dB} / 32768\ \text{LRCK}$<br>`1111`: $0.25\text{ dB} / 65536\ \text{LRCK}$ (Ultra-smooth anti-click) |
| **0** | `DMIC_SENSE` | R/W | `0` | **Digital Microphone PDM Data Sampling Phase:**<br>`0`: Latch DMIC data on **Positive (Rising) Edge** of `DMIC_SCL`<br>`1`: Latch DMIC data on **Negative (Falling) Edge** of `DMIC_SCL` |

---

### Register `0x16` – ADC Sync, Polarity & Digital Scale-Up
* **Address:** `0x16` | **Default:** `0x04` (`0000 0100b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **5** | `ADC_SYNC` | R/W | `0` | **Filter Counter Synchronization with LRCK:**<br>`0`: Optimized for non-standard system clocks<br>`1`: Optimized for standard audio clock rates |
| **4** | `ADC_INV` | R/W | `0` | **ADC Digital Audio Output Polarity:**<br>`0`: Normal non-inverted audio<br>`1`: Invert audio sample phase ($180^\circ$) |
| **3** | `ADC_RAMCLR` | R/W | `0` | **ADC RAM Filter State Clear:** Clears internal filter delay lines when clock starts. |
| **2:0** | `ADC_SCALE[2:0]` | R/W | `100` | **ADC Digital Boost / Scale Up Factor:**<br>`000`: **0 dB**<br>`001`: **+6 dB**<br>`010`: **+12 dB**<br>`011`: **+18 dB**<br>`100`: **+24 dB** (Default)<br>`101`: **+30 dB**<br>`110`: **+36 dB**<br>`111`: **+42 dB** |

---

### Register `0x17` – ADC Digital Volume Control
* **Address:** `0x17` | **Default:** `0xBF` / `0x00` (Note: $0xBF = 0\text{ dB}$)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:0** | `ADC_VOLUME[7:0]` | R/W | `0x00` | **Digital Gain/Attenuation ($0.5\text{ dB}$ per LSB):**<br>`0x00`: **-95.5 dB** (or digital mute)<br>`0x01`: **-95.0 dB**<br>... ($+0.5\text{ dB}$ per increment)<br>`0xBE`: **-0.5 dB**<br>`0xBF`: **0.0 dB (Unity Gain)**<br>`0xC0`: **+0.5 dB**<br>...<br>`0xFF`: **+32.0 dB**<br>*(Note: When ALC is active, this register holds `MAXGAIN` limit)* |

---

### Register `0x18` – ADC Auto Level Control (ALC) & Automute Enable
* **Address:** `0x18` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `ALC_EN` | R/W | `0` | **Auto Level Control (ALC) Enable:**<br>`0`: ALC disabled (Manual gain mode)<br>`1`: ALC active (Dynamically expands weak speech and limits loud peaks) |
| **6** | `ADC_AUTOMUTE_EN`| R/W | `0` | **ADC Hardware Noise Automute Enable:**<br>`0`: Automute disabled<br>`1`: Automute enabled (Mutes or attenuates output when ambient energy falls below noise floor) |
| **3:0** | `ALC_WINSIZE[3:0]`| R/W | `0000` | **ALC Dynamic Response Window Size (Attack/Decay averaging window):**<br>`0000`: $0.25\text{ dB} / 2\ \text{LRCK}$<br>`0001`: $0.25\text{ dB} / 4\ \text{LRCK}$<br>...<br>`1111`: $0.25\text{ dB} / 65536\ \text{LRCK}$ |

---

### Register `0x19` – ALC Target Output Levels (Target Window)
* **Address:** `0x19` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Threshold Table ($0\text{ to }15$) |
|:---:|:---|:---:|:---:|:---|
| **7:4** | `ALC_MAXLEVEL[3:0]`| R/W | `0000` | **Target Maximum Audio Output Level (Peak ceiling):**<br>`0`: $-30.1\text{ dB}$ \| `1`: $-24.1\text{ dB}$ \| `2`: $-20.6\text{ dB}$ \| `3`: $-18.1\text{ dB}$<br>`4`: $-16.1\text{ dB}$ \| `5`: $-14.5\text{ dB}$ \| `6`: $-13.2\text{ dB}$ \| `7`: $-12.0\text{ dB}$<br>`8`: $-11.0\text{ dB}$ \| `9`: $-10.1\text{ dB}$ \| `10`: $-9.3\text{ dB}$ \| `11`: $-8.5\text{ dB}$<br>`12`: $-7.8\text{ dB}$ \| `13`: $-7.2\text{ dB}$ \| `14`: $-6.6\text{ dB}$ \| `15`: **$-6.0\text{ dB}$** |
| **3:0** | `ALC_MINLEVEL[3:0]`| R/W | `0000` | **Target Minimum Audio Output Level (Floor target):**<br>Same scale as `ALC_MAXLEVEL` ($-30.1\text{ dB}$ to $-6.0\text{ dB}$). |

---

### Register `0x1A` – ADC Automute Noise Gate & Window Size
* **Address:** `0x1A` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:4** | `ADC_AUTOMUTE_WS[3:0]`| R/W | `0000` | **Noise Detection Sample Window Duration:**<br>$\text{Samples} = 2^{11} \times (\text{WS} + 1)$<br>`0`: $2048\text{ samples}$ ($42\text{ ms} @ 48\text{ kHz}$)<br>`1`: $4096\text{ samples}$ ($84\text{ ms}$)<br>...<br>`15`: $32768\text{ samples}$ ($688\text{ ms}$) |
| **3:0** | `ADC_AUTOMUTE_NG[3:0]`| R/W | `0000` | **Noise Gate Trigger Level (Threshold below which mic is considered silent):**<br>`0`: $-96\text{ dB}$ \| `1`: $-90\text{ dB}$ \| `2`: $-84\text{ dB}$ \| `3`: $-78\text{ dB}$<br>`4`: $-72\text{ dB}$ \| `5`: $-66\text{ dB}$ \| `6`: $-60\text{ dB}$ \| `7`: $-54\text{ dB}$<br>`8`: $-51\text{ dB}$ \| `9`: $-48\text{ dB}$ \| `10`: $-45\text{ dB}$ \| `11`: $-42\text{ dB}$<br>`12`: $-39\text{ dB}$ \| `13`: $-36\text{ dB}$ \| `14`: $-33\text{ dB}$ \| `15`: $-30\text{ dB}$ |

---

### Register `0x1B` – ADC Automute Attenuation Gain & HPF Stage 1 Coeff
* **Address:** `0x1B` | **Default:** `0x0C` (`0000 1100b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:5** | `ADC_AUTOMUTE_VOL[2:0]`| R/W | `000` | **Mute Attenuation Level:**<br>$\text{Attenuation} = \text{VOL} \times (-4\text{ dB})$<br>`000`: Full Mute to $-\infty\text{ dB}$ (or $0\text{ dB}$ reduction)<br>... ($-4\text{ dB}$ per step down to $-28\text{ dB}$) |
| **4:0** | `ADC_HPFS1[4:0]` | R/W | `01100` | **ADC High-Pass Filter Stage 1 Cutoff Frequency Coefficient.** Sets first-order DC removal filter corner. |

---

### Register `0x1C` – ADC Equalizer Bypass & HPF Stage 2 Control
* **Address:** `0x1C` | **Default:** `0x4C` (`0100 1100b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **6** | `ADC_EQBYPASS` | R/W | `1` | **ADC Parametric Hardware Equalizer (ADCEQ) Bypass:**<br>`0`: ADCEQ enabled (biquad filtering active)<br>`1`: **ADCEQ Bypassed** (Clean linear pass-through, Default) |
| **5** | `ADC_HPF` | R/W | `0` | **ADC DC Offset Dynamic HPF Tracking Mode:**<br>`0`: Freeze calculated DC offset<br>`1`: **Dynamic real-time High-Pass Filter active** |
| **4:0** | `ADC_HPFS2[4:0]` | R/W | `01100` | **ADC High-Pass Filter Stage 2 Cutoff Frequency Coefficient.** |

---

### Registers `0x1D` to `0x30` – ADC 30-Bit Digital Equalizer (ADCEQ) Biquad Coefficients
The ADCEQ implements a 2nd-order IIR biquad transfer function:
$$H(z) = \frac{B_0 + B_1 z^{-1} + B_2 z^{-2}}{1 - A_1 z^{-1} - A_2 z^{-2}}$$
Each coefficient ($B_0, B_1, B_2, A_1, A_2$) is a **30-bit two's complement** value mapped across 4 contiguous registers (MSB to LSB):

| Registers | Coeff Name | Bit Breakdown | Precision |
|:---|:---:|:---|:---:|
| `0x1D` (bits 5:0), `0x1E` (7:0), `0x1F` (7:0), `0x20` (7:0) | **$B_0$** | `ADCEQ_B0[29:0]` | 30-bit signed |
| `0x21` (bits 7:0), `0x22` (7:0), `0x23` (7:0), `0x24` (7:0) | **$A_1$** | `ADCEQ_A1[29:0]` | 30-bit signed |
| `0x25` (bits 7:0), `0x26` (7:0), `0x27` (7:0), `0x28` (7:0) | **$A_2$** | `ADCEQ_A2[29:0]` | 30-bit signed |
| `0x29` (bits 7:0), `0x2A` (7:0), `0x2B` (7:0), `0x2C` (7:0) | **$B_1$** | `ADCEQ_B1[29:0]` | 30-bit signed |
| `0x2D` (bits 7:0), `0x2E` (7:0), `0x2F` (7:0), `0x30` (7:0) | **$B_2$** | `ADCEQ_B2[29:0]` | 30-bit signed |

---

### Register `0x44` – Interconnect, Internal Loopback & ADC Channel Duplication
* **Address:** `0x44` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7** | `ADC2DAC_SEL` | R/W | `0` | **Internal Zero-Latency Hardware Sidetone Loopback:**<br>`0`: Disabled<br>`1`: Direct ADC digital stream routed into DAC (instant local mic monitoring) |
| **6:4** | `ADCDAT_SEL[2:0]`| R/W | `000` | **ASDOUT Serial Data Multiplexer Output Slot Mapping:**<br>`000`: **$ADC + ADC$** (Sends mono mic on **both** Left & Right $I^2S$ slots)<br>`001`: $ADC + 0$ (Mic on Left slot, Zeroes on Right slot)<br>`010`: $0 + ADC$ (Zeroes on Left slot, Mic on Right slot)<br>`011`: $0 + 0$ (Mute both slots)<br>`100`: $DACL + ADC$<br>`101`: $ADC + DACR$<br>`110`: $DACL + DACR$ (DAC loopback monitoring)<br>`111`: Reserved |
| **3** | `I2C_WL` | R/W | `0` | Internal Factory Use. |
| **2:0** | `GPIO_SEL[2:0]` | R/W | `000` | Internal General Purpose Pin Configuration. |

---

### Register `0x45` – General Purpose & Pin Pull-Up Control
* **Address:** `0x45` | **Default:** `0x00` (`0000 0000b`)

| Bit(s) | Bit Name | R/W | Default | Functional Description |
|:---:|:---|:---:|:---:|:---|
| **7:4** | `FORCECSM` | R/W | `0000` | Internal use. |
| **3:1** | `DLY_SEL` | R/W | `000` | Internal delay matching. |
| **0** | `PULLUP_SE` | R/W | `0` | **BCLK / LRCK Internal Pull-Up Resistor Control:**<br>`0`: **Internal Pull-Up Enabled** on BCLK & LRCK<br>`1`: Internal Pull-Up Disabled (Floating inputs/outputs) |

---

### Register `0xFA` – Soft Reset Control
* **Address:** `0xFA` | **Default:** `0x00` (`0000 0000b`)

| Bit | Bit Name | R/W | Default | Description |
|:---:|:---|:---:|:---:|:---|
| **0** | `INI_REG` | R/W | `0` | **Register Soft Factory Reset:** Writing `1` immediately resets all register contents (0x00-0xFF) to power-on default states. |

---

### Register `0xFC` – Hardware Status Flags (Read-Only)
* **Address:** `0xFC` | **Default:** `0x00`

| Bit(s) | Bit Name | Access | Description |
|:---:|:---|:---:|:---|
| **6:4** | `FLAG_CSM_CHIP`| RO | **Internal Chip State Machine State:** `000`=S0 (Power down), `001`=S1, `010`=S2, `011`=S3, `110`=S6, `111`=S7 (Fully Active Running). |
| **1** | `FLAG_ADCAM` | RO | **ADC Automute Active Status Flag:** `1` when noise gate automute is currently triggered. |

---

### Registers `0xFD`, `0xFE`, `0xFF` – Silicon Identification & Revision (Read-Only)
* **`0xFD` (`CHIP_ID1`):** `0x83` (Constant identifier for Everest Semi series)
* **`0xFE` (`CHIP_ID2`):** `0x11` (Identifies `ES8311`)
* **`0xFF` (`CHIP_VER`):** `0x00` (Silicon Revision Code)

---

## 6. Complete Electrical, Power, & Timing Specifications

### 6.1 Absolute Maximum Ratings
* $AVDD, DVDD, PVDD$ to Ground: $-0.3\text{V}$ to $+3.6\text{V}$
* Analog Input Voltage (`MIC1P`, `MIC1N`, `VMID`): $AGND - 0.3\text{V}$ to $AVDD + 0.3\text{V}$
* Digital Input Voltage (`MCLK`, `SCLK`, `LRCK`, `CDATA`, `CCLK`): $DGND - 0.3\text{V}$ to $PVDD + 0.3\text{V}$
* Storage Temperature: $-65^\circ\text{C}$ to $+150^\circ\text{C}$
* Operating Ambient Temperature: $-40^\circ\text{C}$ to $+105^\circ\text{C}$

### 6.2 Recommended Operating Voltages
* Analog ($AVDD$): $1.7\text{V} \le AVDD \le 3.6\text{V}$ (Typ $3.3\text{V}$)
* Digital Core ($DVDD$): $1.6\text{V} \le DVDD \le 3.6\text{V}$ (Typ $1.8\text{V}$ / $3.3\text{V}$)
* I/O Buffers ($PVDD$): $1.6\text{V} \le PVDD \le 3.6\text{V}$ (Typ $3.3\text{V}$)

### 6.3 Digital I/O Switching Timing ($I^2S$ Bus)
* **$MCLK$ Max Frequency:** $49.2\text{ MHz}$ (Duty cycle $40\% - 60\%$).
* **$SCLK$ Max Frequency:** $26\text{ MHz}$.
* **$SCLK$ Pulse Width Low/High ($T_{SLKL}, T_{SCLKH}$):** Min $16\text{ ns}$.
* **$SCLK$ falling to $ASDOUT$ valid delay ($T_{SDO}$):** Max $16\text{ ns}$ (@ $PVDD=3.3\text{V}$), Max $39\text{ ns}$ (@ $PVDD=1.8\text{V}$).
* **$LRCK$ edge to $SCLK$ rising setup time ($T_{LSR}$):** Min $10\text{ ns}$ (in Slave mode).

### 6.4 $I^2C$ Control Bus Timing
* Slave Address: `0011 00x` ($0x18$ if `CE=0`, $0x19$ if `CE=1`).
* Max Clock Frequency ($F_{CCLK}$): $400\text{ kHz}$ (Fast Mode), $100\text{ kHz}$ (Standard).
* Bus Free Time between Stop & Start ($T_{TWID}$): Min $1.3\ \mu\text{s}$.
* Data In Setup Time ($T_{TWDS}$): Min $100\text{ ns}$.
* Data In Hold Time ($T_{TWDH}$): Min $0\text{ ns}$, Max $900\text{ ns}$.

---

## 7. ESP32-S3 Firmware Bring-Up & Initialization Sequence

To configure the ES8311 for analog differential microphone capture and $I^2S$ 24-bit audio forwarding into an ESP32-S3 ($Fs = 48\text{ kHz}$, Master ESP32 / Slave ES8311):

### 7.1 Register Write Sequence

```c
// 1. Trigger Soft Reset to clear dirty state
i2c_write_reg(0xFA, 0x01); // Reset all registers
delay_ms(10);
i2c_write_reg(0xFA, 0x00);

// 2. Power on Reference & VMID charge
i2c_write_reg(0x0D, 0x01); // Start VMID fast pre-charge, enable VREF
i2c_write_reg(0x0E, 0x02); // Low impedance reference buffer
delay_ms(20);              // Wait for VMID decoupling cap to stabilize
i2c_write_reg(0x0D, 0x02); // Switch VMID to normal operating impedance

// 3. Configure Clock Management (Slave Mode, MCLK supplied by ESP32)
i2c_write_reg(0x00, 0x80); // CSM_ON = 1, Slave Mode (MSC = 0), un-reset blocks
i2c_write_reg(0x01, 0x3F); // MCLK_ON=1, BCLK_ON=1, CLKADC_ON=1, ANACLKADC_ON=1
i2c_write_reg(0x02, 0x00); // MCLK pre-divider = 1, mult = 1
i2c_write_reg(0x03, 0x10); // Single speed mode (Fs <= 48kHz), OSR = 64 * Fs
i2c_write_reg(0x05, 0x00); // ADC clock divider = 1

// 4. Configure Serial Data Port Output (ASDOUT to ESP32)
// SDP_OUT_FMT = 00 (I2S Standard), SDP_OUT_WL = 000 (24-bit audio depth)
i2c_write_reg(0x0A, 0x00); 

// 5. Select ADC Routing & Microphone Topology
// LINSEL = 1 (Mic1P - Mic1N differential), DMIC_ON = 0 (Analog Mic), PGA Gain = +18dB (0x6)
i2c_write_reg(0x14, 0x16);

// 6. Enable ADC Analog Blocks
i2c_write_reg(0x0E, 0x00); // PDN_PGA = 0 (PGA ON), PDN_MOD = 0 (Modulator ON)
i2c_write_reg(0x0D, 0x02); // Ensure analog bias & reference active

// 7. Enable High-Pass Filter for DC Offset Removal
i2c_write_reg(0x1C, 0x60); // Dynamic HPF tracking enabled, ADCEQ bypassed (0x40 | 0x20)
i2c_write_reg(0x1B, 0x0C); // HPF Stage 1 corner coefficient

// 8. Configure Digital Gain & Scale
i2c_write_reg(0x16, 0x04); // ADC_SCALE = +24dB digital boost (or 0x00 for 0dB)
i2c_write_reg(0x17, 0xBF); // ADC_VOLUME = 0dB unity gain (0xBF)

// 9. Route Audio on Both I2S Channels (ASDOUT Left + Right duplicate)
i2c_write_reg(0x44, 0x00); // ADCDAT_SEL = 000 (ADC + ADC mono mirrored)

// 10. Start Digital Core State Machine
i2c_write_reg(0x00, 0x80); // Ensure CSM_ON = 1 and digital reset cleared
```

---

## 8. Summary Checklist for Hardware Engineers

| Subsystem | Requirement | Design Check |
|:---|:---|:---|
| **$I^2C$ Bus** | $2.2\text{k}\Omega - 4.7\text{k}\Omega$ pullups on `CCLK` & `CDATA` | Pullups connected to $PVDD$ rail. |
| **Address Pin** | Tie `CE` (Pin 20) cleanly to `GND` or `PVDD` | $0x18$ if low, $0x19$ if high. Never float `CE`. |
| **Decoupling** | $1\ \mu\text{F}$ X7R ceramic caps on `VMID`, `ADCVREF`, `DACVREF` | Place directly adjacent to chip pins; return to `AGND`. |
| **Analog Ground** | Star ground or single solid ground plane with isolation | Connect Exposed Pad (Pin 21) with thermal vias to Ground. |
| **Analog Mic Inputs** | AC coupling series caps ($1\ \mu\text{F}$) on `MIC1P` & `MIC1N` | Match capacitor values and trace lengths to reject common-mode noise. |
| **Unused Pins** | In Mic-only application: `DSDIN` $\rightarrow$ `DGND`; `OUTP`/`OUTN` $\rightarrow$ Open/NC | Keeps DAC quiescent power to zero. |
