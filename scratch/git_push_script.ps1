Remove-Item -Recurse -Force .git -ErrorAction SilentlyContinue

git init
git config user.email "bot@example.com"
git config user.name "AI Assistant"

git remote add origin https://github.com/gupta-tanvi-tft/ESP32-S3-Autorepeat.git
git checkout -b main

# Commit 1
git add CMakeLists.txt
git commit -m "Add CMakeLists.txt"

# Commit 2
git add main/
git commit -m "Add main directory"

# Commit 3
git add ES8311_Audio_Repeater_Arduino.ino
git commit -m "Add Arduino INO file"

# Commit 4
git add ES8311_MICROPHONE_GUIDEBOOK.md
git commit -m "Add microphone guidebook"

# Commit 5
git add ES8311_MICROPHONE_HARDWARE_REFERENCE.md
git commit -m "Add microphone hardware reference"

# Commit 6
git add ES8311_SPEAKER_GUIDEBOOK.md
git commit -m "Add speaker guidebook"

# Commit 7
git add ES8311_SPEAKER_HARDWARE_REFERENCE.md
git commit -m "Add speaker hardware reference"

# Commit 8
git add ESP32S3_ES8311_AUDIO_REPEATER.md
git commit -m "Add main readme for audio repeater"

# Commit 9
git add generate_pdf.py generate_speaker_pdf.py read_serial.py
git commit -m "Add python scripts"

# Commit 10
git add sdkconfig sdkconfig.defaults sdkconfig.old
git commit -m "Add sdkconfig files"

# Commit 11
git add dependencies.lock managed_components/
git commit -m "Add dependencies lock and components"

# Commit 12
# EXCLUDING build/ directory to avoid pushing secrets in compiled files
git add .cache/ serial_output.txt scratch/ *.pdf *.html
git commit -m "Add generated docs and scripts"

git push -u origin main -f
