# ES8311 Microphone Guidebook: A Friendly Guide to Audio Capture

> **Target Audience:** Developers, Embedded Engineers, and Makers working with ESP32 / microcontrollers.  
> **Goal:** Understand how the ES8311 microphone subsystem works, how to wire it up, and how to configure it in software without getting lost in raw register tables.

---

## 📖 1. What is the ES8311?

The **Everest Semiconductor ES8311** is a compact, low-power mono audio CODEC chip. 

In simple terms:
* It takes **sound waves from a microphone** (either analog voltages or digital pulses).
* It **amplifies**, **filters**, and **converts** that sound into clean digital audio numbers (PCM samples).
* It streams those audio numbers to your microcontroller (e.g., **ESP32-S3**) over high-speed digital audio lines ($I^2S$).
* It also includes a speaker/headphone output (DAC), but in this guide, we focus 100% on the **Microphone & Audio Recording (ADC)** side!

```
 ┌──────────┐     Analog Wave / PDM     ┌─────────────┐       I2S Digital PCM       ┌──────────┐
 │Microphone│ ─────────────────────────►│   ES8311    │ ───────────────────────────►│ ESP32-S3 │
 └──────────┘                           │ Audio CODEC │   (Streaming audio data)    └──────────┘
                                        └─────────────┘
                                               ▲
                                               │ I2C (Settings: volume, gain, etc.)
                                               │
                                        ┌──────────┐
                                        │ ESP32-S3 │
                                        └──────────┘
```

---

## 🧠 2. The 6-Stage Audio Pipeline (How Sound Moves Through the Chip)

When someone speaks into the microphone, the audio goes through 6 stages inside the ES8311:

```
  (1) Input Pin ──► (2) Analog PGA ──► (3) ADC Modulator ──► (4) HPF & EQ ──► (5) ALC & Gain ──► (6) I2S Output
   [MIC1P/MIC1N]      [Pre-Amplifier]     [Analog to Dig]     [Clean Noise]    [Auto-Volume]     [ASDOUT Pin]
```

1. **Input Selection:** You choose whether you are using an **Analog Microphone** (Pins 17 & 18) or a **Digital PDM Microphone** (Pin 18 + Clock Pin 6).
2. **Analog Pre-Amplifier (PGA):** If using an analog mic, this boosts weak microphone signals from **$0\text{ dB}$ up to $+30\text{ dB}$** in $3\text{ dB}$ steps.
3. **Delta-Sigma ADC Converter:** Converts the smooth analog voltage into 24-bit digital audio numbers at your chosen sample rate (e.g., $16\text{ kHz}, 44.1\text{ kHz},$ or $48\text{ kHz}$).
4. **Digital Filters (HPF & EQ):**
   * **High-Pass Filter (HPF):** Strips away unwanted DC bias, battery hum, and low-frequency thumps.
   * **Hardware Equalizer (ADCEQ):** Adjusts bass, mids, and treble on-the-fly.
5. **Auto Level Control (ALC) & Noise Gate:**
   * **ALC:** Automatically boosts quiet whisperers and clamps loud shouts so your audio never clips.
   * **Noise Gate:** Automatically mutes the stream when the room is silent so you don't hear background hiss.
6. **I2S Serial Data Forwarder:** Packages the final clean audio and transmits it out of **Pin 7 (`ASDOUT`)** directly into the ESP32 DMA memory.

---

## 🔌 3. Pin Connections Made Simple

The chip comes in a tiny 20-pin square package. Here is how each pin is used when connecting a microphone:

```
                          ┌──────────────┐
              I2C Clock --│ 1  CCLK   CE 20│-- I2C Address (GND=0x18, VDD=0x19)
             Audio Clock --│ 2  MCLK CDAT 19│-- I2C Data
            Power (3.3V) --│ 3  PVDD MICP 18│-- Mic Input (+) / Digital Mic Data
            Power (3.3V) --│ 4  DVDD MICN 17│-- Mic Input (-) [Analog only]
                 Ground --│ 5  DGND VMID 16│-- Mid-rail Bypass Cap (1uF to GND)
         Bit Clock (BCLK) -│ 6  SCLK VREF 15│-- ADC Reference Cap (1uF to GND)
      Audio Out (ASDOUT) -│ 7  ASDO VREF 14│-- DAC Reference Cap (1uF to GND)
      Frame Clock (LRCK) -│ 8  LRCK OUTN 13│-- [Unused / Speaker Output -]
           [DAC Audio In] -│ 9  DSDI OUTP 12│-- [Unused / Speaker Output +]
                 Ground --│ 10 AGND AVDD 11│-- Power (3.3V)
                          └──────────────┘
```

### Quick Pin Category Reference

| Category | Pins | What it does | What to connect to |
|:---|:---|:---|:---|
| **Microphone Inputs** | **Pin 18 (`MIC1P`)**<br>**Pin 17 (`MIC1N`)** | Receives sound waves | **Analog:** Connect to Mic (+) and (-)<br>**Digital:** Pin 18 = Mic Data, Pin 17 = Leave Unused |
| **Audio Stream ($I^2S$)** | **Pin 7 (`ASDOUT`)**<br>**Pin 6 (`SCLK`)**<br>**Pin 8 (`LRCK`)**<br>**Pin 2 (`MCLK`)** | Sends recorded audio numbers to the ESP32 | **ASDOUT** $\rightarrow$ ESP32 I2S Data In<br>**SCLK** $\rightarrow$ ESP32 I2S Bit Clock (BCLK)<br>**LRCK** $\rightarrow$ ESP32 I2S Word Select (WS)<br>**MCLK** $\rightarrow$ ESP32 Master Clock |
| **Control ($I^2C$)** | **Pin 19 (`CDATA`)**<br>**Pin 1 (`CCLK`)**<br>**Pin 20 (`CE`)** | Used to configure volume, gain, and mode | **CDATA** $\rightarrow$ ESP32 I2C SDA (with pull-up resistor)<br>**CCLK** $\rightarrow$ ESP32 I2C SCL (with pull-up resistor)<br>**CE** $\rightarrow$ Tie to GND for address `0x18` (or 3.3V for `0x19`) |
| **Capacitors / Ref** | **Pin 16 (`VMID`)**<br>**Pin 15 (`ADCVREF`)** | Critical analog voltage reference filters | Put a **$1\ \mu\text{F}$ capacitor** between each pin and Analog Ground (`AGND`). **Never leave these floating!** |
| **Power & Ground** | **Pins 3, 4, 11 (`PVDD, DVDD, AVDD`)**<br>**Pins 5, 10, EP (`DGND, AGND, Exposed Pad`)** | Power supply rails | Connect Power pins to clean $3.3\text{V}$ (or $1.8\text{V}$).<br>Connect Ground pins and center pad to Ground. |
| **Playback (Unused)** | **Pin 9 (`DSDIN`)**<br>**Pins 12, 13 (`OUTP, OUTN`)** | Speaker / Headphone playback path | If not using speaker: Tie `DSDIN` to Ground; leave `OUTP` and `OUTN` unconnected. |

---

## 🎙️ 4. The 3 Ways to Wire a Microphone

You can connect three different types of microphones to this chip. Pick the one that matches your hardware:

### Option A: Differential Analog Mic (Best Quality & Noise Immunity)
*Use this if your microphone capsule has two signal wires ($+$ and $-$).*

```
   [Differential Mic]
       MIC (+) ─────||─────► Pin 18 (MIC1P)
                    1uF Cap
       MIC (-) ─────||─────► Pin 17 (MIC1N)
                    1uF Cap
       GND ────────────────► AGND (Ground)
```
* **Why use it:** Rejects electrical noise and power supply buzz from WiFi/Bluetooth transmissions.
* **Software Setting:** Set Register `0x14` to `0x16` (Differential selected, +18dB gain).

---

### Option B: Single-Ended Analog Mic (Standard 2-Wire ECM Capsule)
*Use this with basic 2-pin electret microphone capsules.*

```
   [Single-Ended Mic]
       MIC Signal ──||─────► Pin 18 (MIC1P)
                    1uF Cap
       AGND ────────||─────► Pin 17 (MIC1N)  <-- Ground this side through a 1uF cap!
                    1uF Cap
```
* **Why use it:** Simplest circuit for standard off-the-shelf ECM capsules.
* **Pro Tip:** Don't tie Pin 17 directly to GND—connect it through a matching $1\ \mu\text{F}$ capacitor to GND to keep the amplifier balanced.

---

### Option C: Digital PDM MEMS Mic (Modern Digital Mic)
*Use this with digital MEMS microphones (e.g., MP34DT01, ICS-43434, Knowles PDM).*

```
   [Digital MEMS Mic]
       CLK  ◄─────────────── Pin 6 (DMIC_SCL / SCLK)
       DATA ────────────────► Pin 18 (DMIC_SDA / MIC1P)
       VDD  ◄─────────────── 3.3V
       GND  ────────────────► Ground

       Pin 17 (MIC1N) ─────── (Leave Disconnected / Unused)
```
* **Why use it:** Zero analog noise, no coupling capacitors needed, completely digital pathway.
* **Software Setting:** Set Register `0x14` bit 6 (`DMIC_ON = 1`). Set Register `0x0E` to power down the analog preamp to save battery!

---

## ⚙️ 5. How to Configure the Microphone in 5 Simple Steps

When your microcontroller boots up, follow this friendly 5-step recipe over $I^2C$:

```
 ┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
 │   Step 1    │ ──► │   Step 2    │ ──► │   Step 3    │ ──► │   Step 4    │ ──► │   Step 5    │
 │ Reset Chip  │     │ Power Clocks│     │ Set Format  │     │ Select Mic  │     │ Start Audio │
 └─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

### Step 1: Wake Up & Reset the Chip
* Write `0x01` to Register `0xFA` (Restores clean factory defaults).
* Write `0x00` to Register `0xFA`.
* Write `0x01` to Register `0x0D` (Starts charging the analog reference capacitors).

### Step 2: Configure Clock Management
* Write `0x80` to Register `0x00` (Enables State Machine in Slave Mode).
* Write `0x3F` to Register `0x01` (Turns ON internal MCLK, BCLK, and ADC digital/analog clocks).
* Write `0x10` to Register `0x03` (Sets standard single-speed sampling: $8\text{ kHz} - 48\text{ kHz}$).

### Step 3: Choose Audio Format ($I^2S$ Settings)
* Write `0x00` to Register `0x0A`:
  * Sets format to **Standard $I^2S$**.
  * Sets bit resolution to **24-bit PCM**.
  * Unmutes the serial output line (`ASDOUT`).

### Step 4: Pick Mic Type & Set Volume / Gain
* Write `0x16` to Register `0x14`:
  * Enables Differential Analog Mic (`MIC1P - MIC1N`).
  * Sets Pre-Amplifier (PGA) Gain to **$+18\text{ dB}$** (ideal for speech).
* Write `0xBF` to Register `0x17` (Sets digital volume to $0\text{ dB}$ unity gain).
* Write `0x60` to Register `0x1C` (Enables High-Pass Filter to remove room rumble & DC bias).

### Step 5: Power On Analog Frontend & Stream!
* Write `0x00` to Register `0x0E` (Powers up PGA and Delta-Sigma Modulator).
* Write `0x02` to Register `0x0D` (Switches analog reference to normal running state).
* Write `0x00` to Register `0x44` (Duplicates mono mic audio across both Left and Right $I^2S$ slots).

Audio is now actively streaming out of Pin 7 (`ASDOUT`)!

---

## 🛠️ 6. Troubleshooting & Common Pitfalls

### ❓ Issue 1: "I get no audio at all (silence / all zeroes)."
* **Check the $I^2C$ Address:** Did you ground Pin 20 (`CE`)? If Pin 20 is tied to Ground, the $I^2C$ address is `0x18`. If tied to 3.3V, it is `0x19`.
* **Check MCLK Clock:** The ES8311 requires a Master Clock (`MCLK`) from the ESP32. Ensure your ESP32 $I^2S$ driver is outputting MCLK (typically $256 \times F_s = 12.288\text{ MHz}$ for $48\text{ kHz}$).
* **Check ASDOUT Tri-State:** Verify Register `0x07` bit 4 is `0` (Normal output, not high-impedance).

### ❓ Issue 2: "The recorded audio is extremely quiet."
* **Boost Analog Preamp (PGA):** In Register `0x14`, increase bits 3:0. Try setting gain to `0x8` ($+24\text{ dB}$) or `0xA` ($+30\text{ dB}$).
* **Boost Digital Scale:** In Register `0x16`, bits 2:0 control digital boost from $0\text{ dB}$ up to $+42\text{ dB}$.

### ❓ Issue 3: "There is a loud 50Hz/60Hz hum or WiFi buzz."
* **Missing VMID / VREF Capacitor:** Ensure you placed a $1\ \mu\text{F}$ capacitor on Pin 16 (`VMID`) and Pin 15 (`ADCVREF`) to ground.
* **Switch to Differential:** If using an analog mic near an ESP32 antenna, single-ended wires pick up RF noise. Use a twisted differential pair connected to `MIC1P` and `MIC1N`.
* **Turn ON the High-Pass Filter (HPF):** Ensure Register `0x1C` bit 5 is set to `1` to automatically filter out sub-audio rumble and DC offset.

### ❓ Issue 4: "Audio sounds clipped and distorted when speaking loudly."
* **Lower PGA Gain:** If your mic is very sensitive, drop the PGA gain in Register `0x14` to $+6\text{ dB}$ (`0x2`) or $0\text{ dB}$ (`0x0`).
* **Enable Auto Level Control (ALC):** Turn on ALC in Register `0x18` (`0x80`). The hardware will automatically turn down the volume during loud shouts and turn it up during whispers!

---

## 📊 7. Summary Cheat Sheet

| Parameter | Recommended Typical Value | Register & Bits |
|:---|:---|:---|
| **$I^2C$ Address** | `0x18` (Pin 20 = GND) or `0x19` (Pin 20 = 3.3V) | Pin 20 (`CE`) |
| **Sampling Rate ($F_s$)** | $16\text{ kHz}$ (Voice/Speech) or $48\text{ kHz}$ (High-Fidelity) | Reg `0x03` |
| **Bit Resolution** | $16\text{ bit}$ (`0x0C`) or $24\text{ bit}$ (`0x00`) | Reg `0x0A[4:2]` |
| **Audio Format** | Standard $I^2S$ | Reg `0x0A[1:0] = 00` |
| **PGA Mic Gain** | $+18\text{ dB}$ (`0x6`) to $+24\text{ dB}$ (`0x8`) | Reg `0x14[3:0]` |
| **Digital Volume** | $0.0\text{ dB}$ Unity Gain (`0xBF`) | Reg `0x17` |
| **High Pass Filter** | Active / Dynamic Tracking | Reg `0x1C = 0x60` |
| **Channel Routing** | Dual-Slot Mono Mirror (Left + Right) | Reg `0x44 = 0x00` |
