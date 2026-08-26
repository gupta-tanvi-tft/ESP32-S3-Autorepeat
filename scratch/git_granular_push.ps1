Remove-Item -Recurse -Force .git -ErrorAction SilentlyContinue

git init
git config user.email "bot@example.com"
git config user.name "AI Assistant"
git remote add origin https://github.com/gupta-tanvi-tft/ESP32-S3-Autorepeat.git
git checkout -b main

# Commit 1
git add README.md
git commit -m "Add core project README.md"

# Commit 2
git add ESP32S3_ES8311_AUDIO_REPEATER.md
git commit -m "Add detailed Audio Repeater architecture and wiring guide"

# Commit 3
git add ES8311_MICROPHONE_GUIDEBOOK.md
git commit -m "Add ES8311 Microphone Guidebook"

# Commit 4
git add ES8311_MICROPHONE_HARDWARE_REFERENCE.md
git commit -m "Add ES8311 Microphone Hardware Reference"

# Commit 5
git add ES8311_SPEAKER_GUIDEBOOK.md
git commit -m "Add ES8311 Speaker Guidebook"

# Commit 6
git add ES8311_SPEAKER_HARDWARE_REFERENCE.md
git commit -m "Add ES8311 Speaker Hardware Reference"

# Commit 7
git add CMakeLists.txt
git commit -m "Add root CMakeLists.txt for ESP-IDF"

# Commit 8
git add main/CMakeLists.txt
git commit -m "Add main component CMakeLists.txt"

# Commit 9
git add main/idf_component.yml
git commit -m "Add main component idf_component.yml"

# Commit 10
git add main/stt_client.h
git commit -m "Add STT Client header file"

# Commit 11
git add main/stt_client.c
git commit -m "Add STT Client implementation (Gemini API integration)"

# Commit 12
git add main/es8311_audio_repeater.c
git commit -m "Add ES8311 Audio Repeater I2S full-duplex firmware"

# Commit 13
git add sdkconfig
git commit -m "Add sdkconfig"

# Commit 14
git add sdkconfig.defaults
git commit -m "Add sdkconfig.defaults"

# Commit 15
git add sdkconfig.old
git commit -m "Add sdkconfig.old"

# Commit 16
git add dependencies.lock
git commit -m "Add dependencies.lock"

# Commit 17
git add ES8311_Audio_Repeater_Arduino.ino
git commit -m "Add Arduino IDE compatible sketch for ES8311"

# Commit 18
git add generate_pdf.py generate_speaker_pdf.py
git commit -m "Add python scripts for generating PDF documentation"

# Commit 19
git add read_serial.py
git commit -m "Add python script for reading serial output"

# Commit 20
git add ES8311_Microphone_Fundamentals_Guidebook.pdf ES8311_Speaker_Fundamentals_Guidebook.pdf test.pdf
git commit -m "Add generated PDF guidebooks"

# Commit 21
git add ES8311_Microphone_Fundamentals_Guidebook.html
git commit -m "Add generated HTML guidebook"

# Commit 22
git add scratch/
git commit -m "Add scratch directory with utility scripts"

# Commit 23
git add managed_components/
git commit -m "Add ESP-IDF managed components (led_strip)"

# Commit 24
git add serial_output.txt
git commit -m "Add sample serial output text"

git push -u origin main -f
