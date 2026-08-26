import serial
import sys
import time

try:
    ser = serial.Serial('COM6', 115200, timeout=1)
    print("Opened COM6. Listening for 10 seconds...", flush=True)
    start = time.time()
    with open('serial_output.txt', 'wb') as f:
        while time.time() - start < 10:
            line = ser.readline()
            if line:
                f.write(line)
    ser.close()
    print("Closed. Saved to serial_output.txt", flush=True)
except Exception as e:
    print(f"Error: {e}")
