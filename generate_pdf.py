import pymupdf
import os

pdf_path = r"c:\Users\tanvi-admin\Desktop\ESP32S3_Audio_repeat\ES8311_Microphone_Fundamentals_Guidebook.pdf"

common_style = """
<style>
  body {
    font-family: Helvetica, Arial, sans-serif;
    color: #1e293b;
    background-color: #ffffff;
    line-height: 1.42;
    font-size: 9.5pt;
    margin: 0;
    padding: 0;
  }

  .header-box {
    background-color: #0f172a;
    color: #ffffff;
    padding: 16px 18px;
    border-radius: 6px;
    margin-bottom: 12px;
  }

  .badge {
    background-color: #2563eb;
    color: #ffffff;
    font-size: 7.5pt;
    font-weight: bold;
    padding: 2px 6px;
    border-radius: 3px;
    text-transform: uppercase;
    display: inline-block;
    margin-bottom: 3px;
  }

  h1 {
    font-size: 17pt;
    font-weight: bold;
    color: #ffffff;
    margin: 2px 0 4px 0;
    line-height: 1.2;
  }

  .header-desc {
    color: #cbd5e1;
    font-size: 9.2pt;
    margin-bottom: 8px;
  }

  .meta-table {
    width: 100%;
    border-collapse: collapse;
    border-top: 1px solid #334155;
    margin-top: 6px;
  }

  .meta-table td {
    color: #94a3b8;
    font-size: 8.2pt;
    padding: 4px 0;
    border: none;
  }

  .meta-table td strong {
    color: #38bdf8;
  }

  h2 {
    font-size: 12pt;
    font-weight: bold;
    color: #0f172a;
    border-bottom: 2px solid #2563eb;
    padding-bottom: 3px;
    margin-top: 10px;
    margin-bottom: 8px;
  }

  h3 {
    font-size: 10.5pt;
    font-weight: bold;
    color: #1e3a8a;
    margin-top: 8px;
    margin-bottom: 4px;
  }

  p {
    margin-bottom: 6px;
    color: #334155;
  }

  .info-box {
    background-color: #f8fafc;
    border: 1px solid #cbd5e1;
    border-left: 3px solid #2563eb;
    padding: 8px 10px;
    border-radius: 4px;
    margin: 7px 0;
  }

  .info-box-title {
    font-weight: bold;
    font-size: 9.5pt;
    color: #0f172a;
    margin-bottom: 3px;
  }

  .success-box {
    background-color: #f0fdf4;
    border: 1px solid #bbf7d0;
    border-left: 3px solid #16a34a;
    padding: 8px 10px;
    border-radius: 4px;
    margin: 7px 0;
  }

  .warning-box {
    background-color: #fffbeb;
    border: 1px solid #fde68a;
    border-left: 3px solid #d97706;
    padding: 8px 10px;
    border-radius: 4px;
    margin: 7px 0;
  }

  pre {
    background-color: #0f172a;
    color: #38bdf8;
    font-family: Courier, monospace;
    font-size: 8pt;
    line-height: 1.3;
    padding: 8px 10px;
    border-radius: 5px;
    margin: 6px 0;
    white-space: pre;
  }

  code {
    background-color: #f1f5f9;
    color: #0f172a;
    font-family: Courier, monospace;
    font-size: 8.5pt;
    padding: 1px 3px;
    border-radius: 2px;
    border: 1px solid #cbd5e1;
  }

  table.data-table {
    width: 100%;
    border-collapse: collapse;
    margin: 6px 0 10px 0;
    font-size: 8pt;
  }

  table.data-table th {
    background-color: #1e293b;
    color: #ffffff;
    font-weight: bold;
    padding: 4px 6px;
    text-align: left;
    border: 1px solid #1e293b;
  }

  table.data-table td {
    padding: 3.5px 6px;
    border: 1px solid #cbd5e1;
    color: #334155;
  }

  table.data-table tr:nth-child(even) td {
    background-color: #f8fafc;
  }

  ul, ol {
    margin-left: 16px;
    margin-bottom: 6px;
    color: #334155;
  }

  li {
    margin-bottom: 3px;
  }
</style>
"""

page1_html = f"""<!DOCTYPE html><html><head><meta charset="UTF-8">{common_style}</head><body>
<div class="header-box">
  <div class="badge">Acoustics & Embedded Hardware Guide</div>
  <h1>Microphone & Audio CODEC Fundamentals</h1>
  <div class="header-desc">A complete, crystal-clear visual guide to acoustic sound capture, pre-amps, Delta-Sigma ADCs, digital filtering, and I2S forwarding using the ES8311 and ESP32-S3.</div>
  
  <table class="meta-table">
    <tr>
      <td>Target CODEC: <strong>Everest Semi ES8311 (QFN-20)</strong></td>
      <td>Host Controller: <strong>ESP32-S3 (I2S DMA + I2C)</strong></td>
      <td>Audio Quality: <strong>24-Bit / 100 dB SNR</strong></td>
    </tr>
  </table>
</div>

<h2>1. Core Fundamentals: How Microphones Capture Sound</h2>
<p>Sound is mechanical vibration travelling through air as pressure waves. A <strong>microphone</strong> is an acoustic-to-electrical transducer that converts microscopic sound pressure changes into electrical signals.</p>

<div class="info-box">
  <div class="info-box-title">🎙️ Analog Microphones (Electret ECM & Analog MEMS)</div>
  <p>An analog microphone outputs a continuous AC voltage waveform. Because microphone diaphragms are microscopic, this raw output is tiny—typically between <strong>1 mV and 10 mV RMS</strong>. Without amplification, it is completely inaudible to a microcontroller.</p>
  <p><strong>Challenge:</strong> Weak analog signals easily pick up electrical noise from nearby WiFi antennas, power rails, and switching regulators.</p>
</div>

<div class="info-box">
  <div class="info-box-title">⚡ Digital PDM Microphones (Digital MEMS)</div>
  <p>A digital MEMS microphone integrates the sensor, pre-amp, and an internal 1-bit Delta-Sigma modulator onto a single silicon die. Instead of analog voltage, it outputs a high-frequency <strong>Pulse Density Modulation (PDM)</strong> digital bitstream.</p>
  <p><strong>Benefit:</strong> Highly immune to PCB trace noise and electromagnetic interference (EMI).</p>
</div>

<h2>2. Why Do We Need the ES8311 Audio CODEC?</h2>
<p>Connecting a microphone directly to general-purpose microcontroller ADC pins yields noisy, distorted audio. The <strong>ES8311 Audio CODEC</strong> solves all front-end challenges in hardware:</p>

<table class="data-table">
  <tr>
    <th>Feature</th>
    <th>Microcontroller Built-In ADC (e.g. SAR)</th>
    <th>ES8311 Dedicated Audio CODEC</th>
  </tr>
  <tr>
    <td><strong>Resolution</strong></td>
    <td>12-Bit (High quantization noise)</td>
    <td><strong>24-Bit Multi-Bit Delta-Sigma</strong> (Studio Grade)</td>
  </tr>
  <tr>
    <td><strong>Signal-to-Noise (SNR)</strong></td>
    <td>~55 dB to 65 dB (Audible hiss)</td>
    <td><strong>100 dB SNR</strong> (Crystal Clear)</td>
  </tr>
  <tr>
    <td><strong>PGA Pre-Amplifier</strong></td>
    <td>None (Requires external op-amps)</td>
    <td><strong>Integrated 0 dB to +30 dB PGA</strong> in 3 dB steps</td>
  </tr>
  <tr>
    <td><strong>DC Offset & Hum</strong></td>
    <td>Suffers from DC drift and supply ripple</td>
    <td><strong>Integrated 2-Stage Dynamic HPF</strong></td>
  </tr>
  <tr>
    <td><strong>Dynamic Control</strong></td>
    <td>Must be coded in DSP software</td>
    <td><strong>Hardware Auto Level Control (ALC) + Noise Gate</strong></td>
  </tr>
  <tr>
    <td><strong>CPU Overhead</strong></td>
    <td>High (Constant interrupt handling)</td>
    <td><strong>Zero (Streams via I2S DMA automatically)</strong></td>
  </tr>
</table>
</body></html>"""

page2_html = f"""<!DOCTYPE html><html><head><meta charset="UTF-8">{common_style}</head><body>
<h2>3. Dual-Plane System Architecture: I2C Control vs I2S Data</h2>
<p>The ESP32-S3 communicates with the ES8311 using two independent, dedicated bus interfaces:</p>

<pre>
┌────────────────────────┐                   ┌────────────────────────┐
│     ESP32-S3 MCU       │                   │    ES8311 CODEC        │
│                        │                   │                        │
│  [ I2C Master Engine ] │ ─── I2C Bus ────► │ [ I2C Control Port ]   │
│  (SCL / SDA Lines)     │  (CCLK, CDATA)    │ Set Gain, Vol, Filters │
│                        │                   │                        │
│  [ I2S DMA Engine ]    │ ◄── I2S Bus ───── │ [ I2S Serial Output ]  │
│  (BCLK, WS, DATA_IN)   │  (SCLK,LRCK,ASDO) │ Streams 24-bit PCM     │
└────────────────────────┘                   └────────────────────────┘
</pre>

<ul>
  <li><strong>Control Plane (I2C):</strong> Low-speed configuration bus (100 kHz - 400 kHz). Used once during startup to set volume, pre-amp gain, filter modes, and clock dividers.</li>
  <li><strong>Data Plane (I2S):</strong> High-speed audio streaming bus (2.8 MHz - 12.288 MHz). Streams audio data continuously into ESP32 DMA memory with zero CPU intervention.</li>
</ul>

<h2>4. The 6-Stage Audio Processing Pipeline</h2>
<p>When sound enters the ES8311, it passes through 6 sequential hardware processing stages:</p>

<pre>
 [Acoustic Sound]
       │
       ▼
 [Stage 1: Input MUX] ────► Selects Differential Mic (Pin 18/17) or Digital PDM Mic
       │
       ▼
 [Stage 2: Analog PGA] ───► Programmable Gain: 0 dB to +30 dB (3 dB steps) [Reg 0x14]
       │
       ▼
 [Stage 3: Delta-Sigma] ──► Multi-bit Delta-Sigma ADC oversamples sound at 64x/128x Fs
       │
       ▼
 [Stage 4: HPF & EQ] ─────► 2-stage High-Pass Filter strips DC rumble + 5-band Equalizer
       │
       ▼
 [Stage 5: ALC & Gate] ───► Auto-volume boosts whispers, limits shouts; Noise Gate mutes hiss
       │
       ▼
 [Stage 6: I2S Out (ASDO)]► Serializes 24-bit audio frames out Pin 7 to ESP32 DMA buffer
</pre>
</body></html>"""

page3_html = f"""<!DOCTYPE html><html><head><meta charset="UTF-8">{common_style}</head><body>
<h2>5. Pinout & Pin Role Directory (QFN-20 Package)</h2>

<table class="data-table">
  <tr>
    <th>Pin #</th>
    <th>Pin Name</th>
    <th>Type</th>
    <th>Hardware Description & Connection Guide</th>
  </tr>
  <tr>
    <td><strong>1</strong></td>
    <td><code>CCLK</code></td>
    <td>Input</td>
    <td><strong>I2C Clock:</strong> Connect to ESP32 I2C SCL with a 3.3 kΩ pull-up resistor.</td>
  </tr>
  <tr>
    <td><strong>2</strong></td>
    <td><code>MCLK</code></td>
    <td>Input</td>
    <td><strong>Master Clock:</strong> Sourced from ESP32 (256 × Fs = 12.288 MHz @ 48 kHz).</td>
  </tr>
  <tr>
    <td><strong>3</strong></td>
    <td><code>PVDD</code></td>
    <td>Power</td>
    <td><strong>I/O Pad Power Rail:</strong> Connect to 3.3V with 0.1 µF bypass cap.</td>
  </tr>
  <tr>
    <td><strong>4</strong></td>
    <td><code>DVDD</code></td>
    <td>Power</td>
    <td><strong>Digital Core Power Rail:</strong> Connect to 1.8V or 3.3V with 0.1 µF cap.</td>
  </tr>
  <tr>
    <td><strong>5</strong></td>
    <td><code>DGND</code></td>
    <td>Ground</td>
    <td><strong>Digital Ground:</strong> Tie to main system ground plane.</td>
  </tr>
  <tr>
    <td><strong>6</strong></td>
    <td><code>SCLK / DMIC_SCL</code></td>
    <td>I/O</td>
    <td><strong>Dual Role:</strong> I2S Bit Clock input (slave mode) OR DMIC Clock output for PDM mic.</td>
  </tr>
  <tr>
    <td><strong>7</strong></td>
    <td><code>ASDOUT</code></td>
    <td>Output</td>
    <td><strong>ADC Audio Out:</strong> Streams 24-bit serialized PCM data directly into ESP32 I2S In.</td>
  </tr>
  <tr>
    <td><strong>8</strong></td>
    <td><code>LRCK</code></td>
    <td>I/O</td>
    <td><strong>Frame Clock / Word Select (WS):</strong> Left/Right channel toggle (= Fs = 48 kHz).</td>
  </tr>
  <tr>
    <td><strong>9</strong></td>
    <td><code>DSDIN</code></td>
    <td>Input</td>
    <td><strong>DAC Playback In:</strong> Tie to DGND in recording/mic-only projects.</td>
  </tr>
  <tr>
    <td><strong>10</strong></td>
    <td><code>AGND</code></td>
    <td>Ground</td>
    <td><strong>Analog Ground:</strong> Quiet ground reference for analog input stages.</td>
  </tr>
  <tr>
    <td><strong>11</strong></td>
    <td><code>AVDD</code></td>
    <td>Power</td>
    <td><strong>Analog Power Rail:</strong> 3.3V clean analog power supply with 1 µF || 0.1 µF caps.</td>
  </tr>
  <tr>
    <td><strong>12, 13</strong></td>
    <td><code>OUTP, OUTN</code></td>
    <td>Output</td>
    <td><strong>Speaker / Headphone:</strong> Leave unconnected (NC) in mic-only projects.</td>
  </tr>
  <tr>
    <td><strong>14</strong></td>
    <td><code>DACVREF</code></td>
    <td>Analog Ref</td>
    <td><strong>DAC Voltage Ref:</strong> Connect 1 µF bypass capacitor to AGND.</td>
  </tr>
  <tr>
    <td><strong>15</strong></td>
    <td><code>ADCVREF</code></td>
    <td>Analog Ref</td>
    <td><strong>ADC Voltage Ref:</strong> Connect 1 µF bypass capacitor to AGND. Crucial for SNR!</td>
  </tr>
  <tr>
    <td><strong>16</strong></td>
    <td><code>VMID</code></td>
    <td>Analog Ref</td>
    <td><strong>Mid-Rail DC Bias (AVDD/2):</strong> Connect 1 µF cap to AGND. Do not float!</td>
  </tr>
  <tr>
    <td><strong>17</strong></td>
    <td><code>MIC1N</code></td>
    <td>Input</td>
    <td><strong>Mic Negative Input:</strong> Inverting terminal of analog pre-amp (differential mode).</td>
  </tr>
  <tr>
    <td><strong>18</strong></td>
    <td><code>MIC1P / DMIC_SDA</code></td>
    <td>Input</td>
    <td><strong>Dual Role:</strong> Mic Positive Input (analog mode) OR PDM Data In (digital mic mode).</td>
  </tr>
  <tr>
    <td><strong>19</strong></td>
    <td><code>CDATA</code></td>
    <td>I/O</td>
    <td><strong>I2C Data Line:</strong> Connect to ESP32 I2C SDA with 3.3 kΩ pull-up resistor.</td>
  </tr>
  <tr>
    <td><strong>20</strong></td>
    <td><code>CE</code></td>
    <td>Input</td>
    <td><strong>I2C Address Select:</strong> Tie to GND for address 0x18; tie to 3.3V for address 0x19.</td>
  </tr>
  <tr>
    <td><strong>21 (EP)</strong></td>
    <td><code>Exposed Pad</code></td>
    <td>Ground</td>
    <td><strong>Thermal & Ground Pad:</strong> Solder bottom center pad directly to PCB Ground plane.</td>
  </tr>
</table>

<h2>6. The 3 Hardware Wiring Schemes</h2>

<div class="success-box">
  <div class="info-box-title">Scheme 1: Differential Analog Mic (Best Noise Immunity)</div>
  <pre>
   MIC (+) ──||──[1uF]──► Pin 18 (MIC1P)  |  MIC (-) ──||──[1uF]──► Pin 17 (MIC1N)  |  GND ──► AGND
   * Register: Reg 0x14 = 0x16 (Differential Pair Selected, +18dB PGA Gain)
  </pre>
</div>

<div class="info-box">
  <div class="info-box-title">Scheme 2: Single-Ended Analog Mic (2-Wire ECM)</div>
  <pre>
   Signal ───||──[1uF]──► Pin 18 (MIC1P)  |  AGND ───||──[1uF]──► Pin 17 (MIC1N)
   * Register: Reg 0x14 = 0x16, Reg 0x0E = 0x00
  </pre>
</div>

<div class="warning-box">
  <div class="info-box-title">Scheme 3: Digital PDM MEMS Mic</div>
  <pre>
   CLK ◄── Pin 6 (DMIC_SCL)  |  DATA ──► Pin 18 (DMIC_SDA)  |  VDD ◄── 3.3V  |  GND ──► DGND
   * Register: Reg 0x14[6] = 1 (DMIC_ON), Reg 0x0E = 0x60 (Power Down Analog Preamp)
  </pre>
</div>
</body></html>"""

page4_html = f"""<!DOCTYPE html><html><head><meta charset="UTF-8">{common_style}</head><body>
<h2>7. 5-Step ESP32-S3 Software Bring-Up Recipe</h2>
<p>To start recording 24-bit audio at 48 kHz, send this exact sequence over I2C during system boot:</p>

<pre>
// 1. Soft Reset & Start Reference Voltage Pre-charge
i2c_write(0xFA, 0x01);  // Soft reset all registers to default
delay_ms(10);
i2c_write(0x0D, 0x01);  // Start VMID fast pre-charge
delay_ms(20);           // Wait for VMID bypass capacitor to stabilize
i2c_write(0x0D, 0x02);  // Switch VMID to normal low-noise operating state

// 2. Configure System Clocks (Slave Mode, MCLK from ESP32)
i2c_write(0x00, 0x80);  // Enable State Machine in slave mode
i2c_write(0x01, 0x3F);  // Turn ON MCLK, BCLK, and ADC digital/analog clocks
i2c_write(0x03, 0x10);  // Single speed mode (8-48 kHz), OSR = 64x

// 3. Configure Serial Data Port Output (I2S Format, 24-bit depth)
i2c_write(0x0A, 0x00);  // I2S Standard format, 24-bit word length, unmuted

// 4. Set Analog Mic Routing, PGA Gain (+18dB) & High-Pass Filter
i2c_write(0x14, 0x16);  // Differential Mic mode, PGA Gain = +18 dB
i2c_write(0x1C, 0x60);  // Enable Dynamic High-Pass Filter (removes DC offset)
i2c_write(0x17, 0xBF);  // ADC Digital Volume = 0 dB (Unity Gain)

// 5. Power Up Analog Front-End & Start Streaming
i2c_write(0x0E, 0x00);  // Power up Analog PGA and Delta-Sigma Modulator
i2c_write(0x44, 0x00);  // Duplicate mono mic onto both Left & Right I2S slots
</pre>

<h2>8. Troubleshooting & Cheat Sheet</h2>

<table class="data-table">
  <tr>
    <th>Symptom</th>
    <th>Root Cause</th>
    <th>Hardware / Software Solution</th>
  </tr>
  <tr>
    <td><strong>Dead Silence / All Zeroes</strong></td>
    <td>Wrong I2C Address or missing MCLK</td>
    <td>Verify Pin 20 (<code>CE</code>) is GND (<code>0x18</code>) or 3.3V (<code>0x19</code>). Ensure ESP32 outputs MCLK clock (256 × Fs).</td>
  </tr>
  <tr>
    <td><strong>Audio Very Faint / Quiet</strong></td>
    <td>PGA Gain too low</td>
    <td>Increase PGA Gain in Register <code>0x14[3:0]</code> to <code>0x8</code> (+24 dB) or <code>0xA</code> (+30 dB).</td>
  </tr>
  <tr>
    <td><strong>Loud WiFi / 50Hz Buzz</strong></td>
    <td>Missing VMID cap or single-ended pickup</td>
    <td>Ensure 1 µF capacitor is present on Pin 16 (<code>VMID</code>). Use differential wiring for microphone.</td>
  </tr>
  <tr>
    <td><strong>Distorted / Clipped Audio</strong></td>
    <td>Input signal overdriving PGA</td>
    <td>Lower PGA Gain in Reg <code>0x14</code> to +6 dB, or enable Auto Level Control (ALC) in Reg <code>0x18</code> (<code>0x80</code>).</td>
  </tr>
</table>
</body></html>"""

pages = [page1_html, page2_html, page3_html, page4_html]

# A4 dimensions
page_width = 595.32
page_height = 841.92
margin_h = 36.0
margin_v = 36.0

rect = pymupdf.Rect(margin_h, margin_v, page_width - margin_h, page_height - margin_v)
med_box = pymupdf.Rect(0, 0, page_width, page_height)

writer = pymupdf.DocumentWriter(pdf_path)

for idx, page_html in enumerate(pages):
    story = pymupdf.Story(html=page_html)
    more = 1
    while more:
        device = writer.begin_page(med_box)
        more, filled = story.place(rect)
        story.draw(device)
        writer.end_page()

writer.close()
print(f"Successfully generated clean multi-page PDF: {pdf_path}!")
