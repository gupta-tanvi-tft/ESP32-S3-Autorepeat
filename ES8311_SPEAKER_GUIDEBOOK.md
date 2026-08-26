# ES8311 Speaker & DAC Guidebook: A Friendly Guide to Audio Playback

> **Target Audience:** Embedded Engineers, Firmware Developers, and Hardware Makers working with ESP32 / microcontrollers.  
> **Goal:** Understand how the ES8311 Digital-to-Analog Converter (DAC), speaker output, headphone amplifier, and dynamic range compression work, with simple wiring schematics and clean firmware recipes.

---

## 🔊 1. What is the Speaker & DAC Subsystem?

While the microphone subsystem turns physical sound into digital numbers, the **Speaker & DAC Subsystem** does the exact opposite:

* It receives **digital audio numbers (PCM samples)** from your microcontroller (e.g., **ESP32-S3**) over high-speed $I^2S$ audio lines.
* It **shapes**, **equalizes**, and **converts** those digital numbers back into a smooth analog electrical wave with **110 dB Signal-to-Noise Ratio (SNR)**.
* It drives that analog wave out through its internal amplifiers to **headphones** or an **external Class-D power amplifier** to play loud and clear sound through a loudspeaker!

```
 ┌──────────┐      I2S Digital PCM Stream      ┌─────────────┐       Analog Audio Waves       ┌────────────────┐      Acoustic Sound      ┌───────────┐
 │ ESP32-S3 │ ────────────────────────────────►│   ES8311    │ ──────────────────────────────►│ Class-D Power  │ ────────────────────────►│  Speaker  │
 └──────────┘       (Music / Voice Data)       │  Audio DAC  │    (OUTP & OUTN Differential)  │ Amplifier Chip │     (Voice / Music)      │ (4Ω / 8Ω) │
      ▲                                        └─────────────┘                                └────────────────┘                          └───────────┘
      │                     I2C Control               ▲
      └───────────────────────────────────────────────┘
            (Volume, DRC Limiter, Anti-Pop Ramping)
```

---

## 🧠 2. The 7-Stage Playback Signal Journey

When your ESP32 streams an audio track or voice prompt, the data travels through 7 hardware blocks inside the ES8311:

```
  (1) DSDIN Pin ──► (2) Channel Mux ──► (3) DACEQ Filter ──► (4) Volume ──► (5) DRC Limiter ──► (6) Delta-Sigma DAC ──► (7) Output Driver
   [I2S In]          [Left/Right Slot]    [Equalizer]         [-95 to +32dB]   [Anti-Clipping]      [Dig to Analog]       [OUTP & OUTN]
```

1. **Digital Audio Input (`DSDIN` - Pin 9):** Receives 16-bit, 24-bit, or 32-bit PCM audio frames synchronized to the Bit Clock (`SCLK`) and Frame Clock (`LRCK`).
2. **Channel Slot Selector (Reg `0x09`):** Because ES8311 is a high-performance mono DAC, you can choose whether it plays the **Left Channel** or **Right Channel** of your stereo $I^2S$ stream.
3. **Hardware Parametric Equalizer (DACEQ - Reg `0x37`–`0x43`):** 30-bit precision hardware biquad filter that lets you boost bass or tune vocal frequencies without burning any ESP32 CPU cycles!
4. **Digital Volume Control (Reg `0x32`):** Smooth master volume control from $-95.5\text{ dB}$ (mute) up to $+32.0\text{ dB}$ boost in fine $0.5\text{ dB}$ steps.
5. **Dynamic Range Compression & Limiter (DRC - Reg `0x34`, `0x35`):** Automatically squashes loud audio spikes to protect your speaker from distortion and blowing out.
6. **Multi-Bit Delta-Sigma Modulator (Reg `0x04`):** Reconstructs the digital stream into continuous analog voltages with studio-grade $110\text{ dB}$ dynamic range.
7. **Analog Output Stage (`OUTP` & `OUTN` - Pins 12 & 13):** High-linearity differential driver capable of feeding power amps or driving low-impedance headphones.

---

## 🔌 3. Playback Pin Connections Made Simple

Here is how every playback pin on the 20-pin chip is connected:

```
                          ┌──────────────┐
              I2C Clock --│ 1  CCLK   CE 20│-- I2C Address (GND=0x18, VDD=0x19)
            Audio MCLK ---│ 2  MCLK CDAT 19│-- I2C Data (with 3.3k pullup)
            Power (3.3V) -│ 3  PVDD MICP 18│-- [Mic Pin - Not needed for playback]
            Power (3.3V) -│ 4  DVDD MICN 17│-- [Mic Pin - Not needed for playback]
                 Ground --│ 5  DGND VMID 16│-- Mid-rail Bias Cap (1uF to AGND)
         Bit Clock (BCLK) -│ 6  SCLK VREF 15│-- [ADCVREF Cap - 1uF to AGND]
            [ASDOUT Mic] -│ 7  ASDO VREF 14│-- DAC Reference Cap (1uF to AGND)
      Frame Clock (LRCK) -│ 8  LRCK OUTN 13│-- Speaker Output Negative (OUTN)
      Audio In (DSDIN) ---│ 9  DSDI OUTP 12│-- Speaker Output Positive (OUTP)
                 Ground --│ 10 AGND AVDD 11│-- Analog Power (3.3V)
                          └──────────────┘
```

### Quick Pin Category Table

| Category | Pins | What it does | Where to wire it |
|:---|:---|:---|:---|
| **Audio Stream In ($I^2S$)** | **Pin 9 (`DSDIN`)**<br>**Pin 6 (`SCLK`)**<br>**Pin 8 (`LRCK`)**<br>**Pin 2 (`MCLK`)** | Receives PCM audio stream from ESP32 | **DSDIN** $\leftarrow$ ESP32 I2S Data Out<br>**SCLK** $\leftarrow$ ESP32 I2S Bit Clock (BCLK)<br>**LRCK** $\leftarrow$ ESP32 I2S Word Select (WS)<br>**MCLK** $\leftarrow$ ESP32 Master Clock ($256 \times Fs$) |
| **Speaker Outputs** | **Pin 12 (`OUTP`)**<br>**Pin 13 (`OUTN`)** | Analog audio wave outputs | **Differential:** Connect to `IN+` and `IN-` of power amp via $1\ \mu\text{F}$ caps.<br>**Headphones:** Connect via $100\ \mu\text{F} - 220\ \mu\text{F}$ caps. |
| **Control ($I^2C$)** | **Pin 19 (`CDATA`)**<br>**Pin 1 (`CCLK`)**<br>**Pin 20 (`CE`)** | Sets volume, DRC limiter, and enables DAC | **CDATA** $\leftrightarrow$ ESP32 SDA (with $3.3\text{k}\Omega$ pullup)<br>**CCLK** $\leftarrow$ ESP32 SCL (with $3.3\text{k}\Omega$ pullup)<br>**CE** $\rightarrow$ Tie to GND (Address `0x18`) or 3.3V (`0x19`) |
| **Critical Capacitors** | **Pin 14 (`DACVREF`)**<br>**Pin 16 (`VMID`)** | Internal DAC reference filter & mid-rail DC bias | Connect a **$1\ \mu\text{F}$ ceramic capacitor** from each pin to `AGND`. **Never float these pins!** |
| **Power Rails** | **Pins 3, 4, 11 (`PVDD, DVDD, AVDD`)**<br>**Pins 5, 10, EP (`DGND, AGND, Exposed Pad`)** | Powers digital core, I/O pads, and analog DAC | Connect Power to $3.3\text{V}$ (or $1.8\text{V}$). Solder Exposed Center Pad directly to PCB Ground plane. |

---

## 🔊 4. The 3 Speaker Wiring Schematics

### Scheme 1: Differential Speaker Output with Power Amp (Loudspeaker Mode)
*Use this for smart speakers, SoundBoxes, toys, and IoT alarms requiring loud volume.*

```
   ES8311 CODEC                          Class-D Power Amp (e.g. NS4168/MAX98357)       Speaker
  ┌──────────────┐                             ┌────────────────────────┐             ┌─────────┐
  │  Pin 12 OUTP ├───||───[ 1 uF Cap ]────────►│ IN+                VO+ ├────────────►│ SPK (+) │
  │  Pin 13 OUTN ├───||───[ 1 uF Cap ]────────►│ IN-                VO- ├────────────►│ SPK (-) │
  │  Pin 10 AGND ├────────────────────────────►│ GND                    │             │ 4Ω / 8Ω │
  └──────────────┘                             └────────────────────────┘             └─────────┘
```
* **Why it's the best:** Differential wiring cancels WiFi RF bursts and power buzz, and delivers **$2\times$ voltage swing** into the amplifier input!
* **Software Setting:** Set Register `0x13` to `0x00` (Line Out Mode).

---

### Scheme 2: Direct Headphone / Earphone Drive (No External Amp Needed)
*Use this for direct connection to $16\Omega$ or $32\Omega$ earbuds and headphones.*

```
   ES8311 CODEC                                 3.5mm Headphone Jack
  ┌──────────────┐                                ┌─────────────────────────┐
  │  Pin 12 OUTP ├───||───[ 100 uF - 220 uF ]────►│ Tip (Left / Mono)       │
  │  Pin 13 OUTN ├───||───[ 100 uF - 220 uF ]────►│ Ring (Right Channel)    │
  │  Pin 10 AGND ├───────────────────────────────►│ Sleeve (Ground)         │
  └──────────────┘                                └─────────────────────────┘
```
* **Pro Tip:** Use large $100\ \mu\text{F}$ to $220\ \mu\text{F}$ electrolytic/tantalum capacitors to preserve rich, deep bass response down to $20\text{ Hz}$.
* **Software Setting:** Set Register `0x13` to `0x10` (`HPSW = 1`) to turn on the high-current headphone driver.

---

### Scheme 3: Single-Ended Line Out (Aux Out to PC / Mixer)
*Use this for auxiliary audio out or line-in to an external mixer ($10\text{ k}\Omega$ load).*

```
   ES8311 CODEC                                 Aux Jack
  ┌──────────────┐                                ┌─────────────────────────┐
  │  Pin 12 OUTP ├───||───[ 1 uF Cap ]───────────►│ Line Out Signal         │
  │  Pin 13 OUTN ├──── (Leave Open / NC)          │                         │
  │  Pin 10 AGND ├───────────────────────────────►│ Ground Shield           │
  └──────────────┘                                └─────────────────────────┘
```

---

## ⚙️ 5. How to Configure Speaker Playback in 5 Simple Steps

When your microcontroller boots, send this simple 5-step sequence over $I^2C$:

```
 ┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
 │   Step 1    │ ──► │   Step 2    │ ──► │   Step 3    │ ──► │   Step 4    │ ──► │   Step 5    │
 │ Reset Chip  │     │ Power Clocks│     │ Set Format  │     │ Anti-Pop/Vol│     │ Power On DAC│
 └─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

### Step 1: Wake Up & Pre-charge Reference Voltages
* Write `0x01` to Register `0xFA` (Soft reset to clean defaults).
* Write `0x01` to Register `0x0D` (Starts fast pre-charge on `VMID` and `DACVREF` capacitors).
* Wait $25\text{ ms}$ for voltages to stabilize, then write `0x02` to Register `0x0D`.

### Step 2: Configure System Clocks
* Write `0x80` to Register `0x00` (Enables State Machine in Slave Mode).
* Write `0x35` to Register `0x01` (Turns ON MCLK, BCLK, DAC digital clock, and DAC analog modulator).
* Write `0x10` to Register `0x04` (Sets DAC oversampling to $64 \times Fs$).

### Step 3: Configure $I^2S$ Audio Input
* Write `0x00` to Register `0x09`:
  * Plays **Left Channel** of $I^2S$ audio.
  * Sets word depth to **24-bit PCM** (or write `0x0C` for 16-bit).
  * Sets protocol to **Standard $I^2S$** and un-mutes input.

### Step 4: Anti-Pop Soft Ramping & Master Volume
* Write `0x88` to Register `0x37` (Enables smooth anti-pop soft ramping so you never hear loud startup clicks).
* Write `0xBF` to Register `0x32` (Sets Master Volume to $0\text{ dB}$ Unity Gain = 100% Volume).

### Step 5: Power On Analog DAC Output
* Write `0x00` to Register `0x13` (Selects Line Out mode; use `0x10` for Headphones).
* Write `0x00` to Register `0x12` (Powers ON DAC analog core and enables reference output).

Your speaker is now active and playing audio streamed from the ESP32!

---

## 🛠️ 6. Troubleshooting & Common Pitfalls

### ❓ Issue 1: "I hear a loud 'POP' sound when playback starts or stops."
* **Enable Soft Ramping:** In Register `0x37`, set bits 7:4 to `0x8` ($0.25\text{ dB} / 512\ \text{LRCK}$). The hardware will smoothly ramp the audio up and down rather than abruptly starting.
* **Wait for VMID to Charge:** Ensure your firmware gives the `VMID` capacitor at least $25\text{ ms}$ to pre-charge before turning on the DAC power in Register `0x12`.

### ❓ Issue 2: "The speaker output is completely silent."
* **Check Register 0x12 (DAC Power):** Both bit 1 (`PDN_DAC = 0`) and bit 0 (`ENREFR = 1`) must be configured to enable the analog output circuits.
* **Check Register 0x32 (Volume):** The default power-on value is `0x00` ($-95.5\text{ dB}$ digital silence). You **must** write `0xBF` ($0\text{ dB}$) to hear sound!
* **Check I2S Channel Slot (Reg 0x09):** If your audio file has sound only on the Right channel, set Reg `0x09` bit 7 to `1` (`SDP_IN_SEL = 1`).

### ❓ Issue 3: "Audio is distorted and raspy at high volume."
* **Turn ON the DRC Peak Limiter:** Write `0x80` to Register `0x34` (`DRC_EN = 1`) and set Reg `0x35` to `0xF0` ($-6\text{ dB}$ peak limit). The CODEC will automatically compress loud peaks and prevent clipping distortion!
* **Check Series DC Blocking Caps:** Ensure $1\ \mu\text{F}$ capacitors are present between `OUTP`/`OUTN` and your power amplifier to block DC bias current.

---

## 📊 7. Summary Playback Cheat Sheet

| Parameter | Recommended Setting | Register & Bitfields |
|:---|:---|:---|
| **$I^2C$ Address** | `0x18` (Pin 20 = GND) or `0x19` (Pin 20 = 3.3V) | Pin 20 (`CE`) |
| **Audio Format** | Standard $I^2S$, 24-bit (or 16-bit) | Reg `0x09 = 0x00` (24-bit) / `0x0C` (16-bit) |
| **Master Volume** | $0.0\text{ dB}$ Unity Gain (100%) | Reg `0x32 = 0xBF` |
| **Anti-Pop Slew Rate** | $0.25\text{ dB} / 512\ \text{LRCK}$ | Reg `0x37 = 0x88` |
| **Output Driver Mode** | Line Out (`0x00`) / Headphone (`0x10`) | Reg `0x13[4]` (`HPSW`) |
| **DAC Power State** | Active & Reference Enabled | Reg `0x12 = 0x00` (`PDN_DAC=0, ENREFR=1`) |
| **DRC Peak Limiter** | Active ($-6\text{ dB}$ ceiling) | Reg `0x34 = 0x80`, Reg `0x35 = 0xF0` |
| **Mic-to-Speaker Sidetone**| Direct Zero-Latency Loopback | Reg `0x44[7] = 1` (`ADC2DAC_SEL = 1`) |
