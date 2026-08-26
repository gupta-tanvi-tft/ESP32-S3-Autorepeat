import pymupdf
import os

pdf_path = r"c:\Users\tanvi-admin\Desktop\ESP32S3_Audio_repeat\ES8311_Speaker_Fundamentals_Guidebook.pdf"

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
  <div class="badge">Acoustics & Embedded Playback Guide</div>
  <h1>Speaker & Audio DAC Fundamentals</h1>
  <div class="header-desc">A complete, crystal-clear guide to digital audio playback, Delta-Sigma DAC reconstruction, Dynamic Range Compression (DRC), anti-pop ramping, and speaker amplification using the ES8311 and ESP32-S3.</div>
  
  <table class="meta-table">
    <tr>
      <td>Target CODEC: <strong>Everest Semi ES8311 (QFN-20)</strong></td>
      <td>Host Controller: <strong>ESP32-S3 (I2S DMA + I2C)</strong></td>
      <td>Audio Quality: <strong>24-Bit / 110 dB SNR</strong></td>
    </tr>
  </table>
</div>

<h2>1. Core Fundamentals: How Digital Audio Becomes Sound</h2>
<p>Digital sound stored in memory is a stream of numeric PCM samples. The <strong>Speaker & DAC subsystem</strong> reconstructs these numbers into continuous analog electrical waves that push a physical speaker cone back and forth to create sound in the air.</p>

<div class="info-box">
  <div class="info-box-title">🔊 Loudspeaker & Power Amp Driving (Differential BTL)</div>
  <p>Loudspeakers require significant electrical current (e.g. 1W to 3W). The ES8311 acts as a high-fidelity pre-driver, outputting clean differential signals (<code>OUTP</code> and <code>OUTN</code>) to an external Class-D power amplifier (such as NS4168, MAX98357, or NS4150).</p>
  <p><strong>Advantage:</strong> Differential output doubles voltage swing (+6 dB) and cancels WiFi RF noise and power supply hum.</p>
</div>

<div class="info-box">
  <div class="info-box-title">🎧 Direct Headphone / Earphone Driving (Single-Ended)</div>
  <p>The ES8311 contains an on-chip, low-impedance headphone driver. By setting <code>HPSW = 1</code> in Register <code>0x13</code>, it can directly drive 16Ω to 32Ω earphones through DC-blocking capacitors without requiring an external amplifier.</p>
</div>

<h2>2. Why Do We Need the ES8311 Audio DAC?</h2>
<p>Driving speakers directly from microcontroller PWM or internal 8-bit DACs produces thin, noisy, distorted audio. The <strong>ES8311 Audio CODEC</strong> provides studio-grade playback in hardware:</p>

<table class="data-table">
  <tr>
    <th>Playback Feature</th>
    <th>Microcontroller Built-In DAC / PWM</th>
    <th>ES8311 Dedicated Audio DAC</th>
  </tr>
  <tr>
    <td><strong>Resolution</strong></td>
    <td>8-Bit / 10-Bit (High quantization noise)</td>
    <td><strong>24-Bit Multi-Bit Delta-Sigma</strong> (Studio Quality)</td>
  </tr>
  <tr>
    <td><strong>Signal-to-Noise (SNR)</strong></td>
    <td>~50 dB to 60 dB (Loud hiss & whine)</td>
    <td><strong>110 dB SNR</strong> (Ultra-quiet black background)</td>
  </tr>
  <tr>
    <td><strong>THD+N Distortion</strong></td>
    <td>~1% to 5% (Distorted bass & highs)</td>
    <td><strong>-80 dB / 0.01% THD+N</strong> (Pristine clarity)</td>
  </tr>
  <tr>
    <td><strong>Anti-Pop & Click</strong></td>
    <td>Loud pop on speaker when boot/shutdown</td>
    <td><strong>Hardware Soft Volume Slew-Rate Ramping</strong></td>
  </tr>
  <tr>
    <td><strong>Dynamic Control</strong></td>
    <td>Must be coded in complex DSP software</td>
    <td><strong>Hardware Dynamic Range Compression (DRC)</strong></td>
  </tr>
  <tr>
    <td><strong>CPU Overhead</strong></td>
    <td>High CPU load during timer interrupts</td>
    <td><strong>Zero (Streams via I2S DMA automatically)</strong></td>
  </tr>
</table>
</body></html>"""

page2_html = f"""<!DOCTYPE html><html><head><meta charset="UTF-8">{common_style}</head><body>
<h2>3. Dual-Plane System Architecture: I2C Control vs I2S Playback</h2>
<p>The ESP32-S3 communicates with the ES8311 using two dedicated, concurrent interfaces:</p>

<pre>
┌────────────────────────┐                   ┌────────────────────────┐
│     ESP32-S3 MCU       │                   │    ES8311 CODEC        │
│                        │                   │                        │
│  [ I2C Master Engine ] │ ─── I2C Bus ────► │ [ I2C Control Port ]   │
│  (SCL / SDA Lines)     │  (CCLK, CDATA)    │ Set Vol, DRC, Anti-Pop │
│                        │                   │                        │
│  [ I2S DMA Engine ]    │ ─── I2S Bus ────► │ [ I2S Audio In (DSDI)] │
│  (BCLK, WS, DATA_OUT)  │  (SCLK,LRCK,DSDI) │ Plays 24-bit PCM stream│
└────────────────────────┘                   └────────────────────────┘
</pre>

<ul>
  <li><strong>Control Plane (I2C):</strong> Low-speed configuration bus (100 kHz - 400 kHz). Adjusts volume, enables DRC compression, and configures anti-pop ramps.</li>
  <li><strong>Data Plane (I2S):</strong> High-speed audio streaming bus. Pushes PCM audio frames continuously from ESP32 flash memory or network streams into the DAC with zero CPU latency.</li>
</ul>

<h2>4. The 7-Stage Playback Signal Processing Pipeline</h2>
<p>When digital audio enters the ES8311, it passes through 7 hardware processing stages:</p>

<pre>
 [ESP32-S3 I2S Playback Stream]
       │
       ▼
 [Stage 1: DSDIN Pin 9] ───► Receives 16/24/32-bit PCM audio frames over I2S
       │
       ▼
 [Stage 2: Channel Mux] ───► Selects Left or Right channel slot from stereo stream [Reg 0x09]
       │
       ▼
 [Stage 3: DACEQ Filter] ──► 30-bit precision hardware parametric biquad equalizer [Reg 0x38-0x43]
       │
       ▼
 [Stage 4: Digital Volume] ─► Master volume control: -95.5 dB to +32 dB in 0.5 dB steps [Reg 0x32]
       │
       ▼
 [Stage 5: DRC Limiter] ───► Dynamic Range Compressor squashes loud peaks to prevent clipping [Reg 0x34]
       │
       ▼
 [Stage 6: Delta-Sigma DAC]► Multi-bit Delta-Sigma Modulator (64x - 508x Fs) reconstructs analog wave
       │
       ▼
 [Stage 7: Output Driver] ─► Drives OUTP & OUTN to external Class-D amp or headphones [Reg 0x12, 0x13]
</pre>
</body></html>"""

page3_html = f"""<!DOCTYPE html><html><head><meta charset="UTF-8">{common_style}</head><body>
<h2>5. Pinout & Pin Role Directory for Playback (QFN-20)</h2>

<table class="data-table">
  <tr>
    <th>Pin #</th>
    <th>Pin Name</th>
    <th>Type</th>
    <th>Playback Hardware Description & Connection Guide</th>
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
    <td><code>SCLK</code></td>
    <td>Input</td>
    <td><strong>I2S Bit Clock:</strong> Synchronizes incoming PCM bits on DSDIN.</td>
  </tr>
  <tr>
    <td><strong>8</strong></td>
    <td><code>LRCK</code></td>
    <td>Input</td>
    <td><strong>Frame Clock / Word Select (WS):</strong> Left/Right channel frame toggle (= Fs = 48 kHz).</td>
  </tr>
  <tr>
    <td><strong>9</strong></td>
    <td><code>DSDIN</code></td>
    <td>Input</td>
    <td><strong>DAC Serial Audio Data In:</strong> Connect to ESP32 I2S Data Out pin.</td>
  </tr>
  <tr>
    <td><strong>10</strong></td>
    <td><code>AGND</code></td>
    <td>Ground</td>
    <td><strong>Analog Ground:</strong> Quiet ground reference for analog DAC and output buffers.</td>
  </tr>
  <tr>
    <td><strong>11</strong></td>
    <td><code>AVDD</code></td>
    <td>Power</td>
    <td><strong>Analog Power Rail:</strong> 3.3V clean analog supply with 1 µF || 0.1 µF bypass caps.</td>
  </tr>
  <tr>
    <td><strong>12</strong></td>
    <td><code>OUTP</code></td>
    <td>Output</td>
    <td><strong>Positive Analog Audio Output:</strong> Connect to IN+ of power amp or Left earphone.</td>
  </tr>
  <tr>
    <td><strong>13</strong></td>
    <td><code>OUTN</code></td>
    <td>Output</td>
    <td><strong>Negative Analog Audio Output:</strong> Connect to IN- of power amp (Differential 180° phase).</td>
  </tr>
  <tr>
    <td><strong>14</strong></td>
    <td><code>DACVREF</code></td>
    <td>Analog Ref</td>
    <td><strong>DAC Voltage Ref:</strong> Connect 1 µF bypass capacitor to AGND. Essential for 110 dB SNR!</td>
  </tr>
  <tr>
    <td><strong>16</strong></td>
    <td><code>VMID</code></td>
    <td>Analog Ref</td>
    <td><strong>Mid-Rail DC Bias (AVDD/2):</strong> Connect 1 µF cap to AGND. Prevents startup pop noise.</td>
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

<h2>6. The 3 Speaker & Output Wiring Schematics</h2>

<div class="success-box">
  <div class="info-box-title">Scheme 1: Differential Speaker Output with Power Amp (Loudspeakers)</div>
  <pre>
   OUTP (Pin 12) ───||───[ 1uF Cap ]───► Power Amp IN+  |  OUT+ ──► Speaker (+)
   OUTN (Pin 13) ───||───[ 1uF Cap ]───► Power Amp IN-  |  OUT- ──► Speaker (-) [4Ω / 8Ω]
   * Register: Reg 0x13 = 0x00 (Line Out Mode)
  </pre>
</div>

<div class="info-box">
  <div class="info-box-title">Scheme 2: Direct Headphone / Earphone Drive (16Ω - 32Ω)</div>
  <pre>
   OUTP (Pin 12) ───||───[ 100uF - 220uF ]───► 3.5mm Tip (Left/Mono)
   OUTN (Pin 13) ───||───[ 100uF - 220uF ]───► 3.5mm Ring (Right) | Sleeve ──► AGND
   * Register: Reg 0x13 = 0x10 (Enable Headphone High-Current Driver Mode)
  </pre>
</div>

<div class="warning-box">
  <div class="info-box-title">Scheme 3: Single-Ended Line Out (Aux Out to PC / Mixer)</div>
  <pre>
   OUTP (Pin 12) ───||───[ 1uF Cap ]───► Aux Signal Line Out | AGND ──► Shield / Ground
   * Pin 13 (OUTN) is left unconnected (NC).
  </pre>
</div>
</body></html>"""

page4_html = f"""<!DOCTYPE html><html><head><meta charset="UTF-8">{common_style}</head><body>
<h2>7. 5-Step ESP32-S3 Playback Bring-Up Recipe</h2>
<p>To start playing 24-bit audio at 48 kHz, send this exact sequence over I2C during system boot:</p>

<pre>
// 1. Soft Reset & Start Reference Voltage Pre-charge
i2c_write(0xFA, 0x01);  // Soft reset all registers to default
delay_ms(10);
i2c_write(0x0D, 0x01);  // Start VMID & DACVREF fast pre-charge
delay_ms(25);           // Wait for capacitors to stabilize (prevents pop click)
i2c_write(0x0D, 0x02);  // Switch VMID to normal low-noise operating state

// 2. Configure System Clocks (Slave Mode, MCLK from ESP32)
i2c_write(0x00, 0x80);  // Enable State Machine in slave mode
i2c_write(0x01, 0x35);  // Turn ON MCLK, BCLK, DAC digital clock, and DAC analog clock
i2c_write(0x04, 0x10);  // DAC Oversampling Ratio = 64x (or 0x40 for 256x)

// 3. Configure Serial Data Port Input (I2S Format, 24-bit depth)
i2c_write(0x09, 0x00);  // SDP_IN: Play Left channel, 24-bit word length, I2S, unmuted

// 4. Configure Anti-Pop Soft Ramping & Master Volume
i2c_write(0x37, 0x88);  // 0.25 dB / 512 LRCK slew rate, DACEQ bypassed
i2c_write(0x32, 0xBF);  // Master Volume = 0 dB Unity Gain (100% Volume)

// 5. Power Up Analog DAC Output Stage
i2c_write(0x13, 0x00);  // HPSW = 0 (Line Out mode for external speaker amp)
i2c_write(0x12, 0x00);  // Power ON DAC analog core and enable reference output
</pre>

<h2>8. Speaker Troubleshooting & Cheat Sheet</h2>

<table class="data-table">
  <tr>
    <th>Symptom</th>
    <th>Root Cause</th>
    <th>Hardware / Software Solution</th>
  </tr>
  <tr>
    <td><strong>Loud Pop on Startup / Shutdown</strong></td>
    <td>Abrupt power toggle without pre-charge</td>
    <td>Allow VMID 25 ms to charge before enabling DAC (Reg <code>0x12</code>). Enable soft ramping in Reg <code>0x37 = 0x88</code>.</td>
  </tr>
  <tr>
    <td><strong>Complete Silence</strong></td>
    <td>Volume default is mute (0x00)</td>
    <td>Register <code>0x32</code> defaults to -95.5 dB. You must write <code>0xBF</code> (0 dB) to enable sound. Verify Reg <code>0x12 = 0x00</code>.</td>
  </tr>
  <tr>
    <td><strong>Distorted / Raspy at Loud Volume</strong></td>
    <td>Input audio clipping speaker amp</td>
    <td>Enable DRC Compressor: write <code>0x80</code> to Reg <code>0x34</code> and <code>0xF0</code> to Reg <code>0x35</code> (-6 dB peak limit).</td>
  </tr>
  <tr>
    <td><strong>Direct Mic-to-Speaker Sidetone</strong></td>
    <td>Need zero-latency local mic monitoring</td>
    <td>Set Reg <code>0x44[7] = 1</code> (<code>ADC2DAC_SEL = 1</code>) to stream microphone directly into speaker in hardware.</td>
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
print(f"Successfully generated clean speaker PDF: {pdf_path}!")
