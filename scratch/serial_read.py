import serial
import time

try:
    ser = serial.Serial('COM6', 115200, timeout=1)
    print("Connected to COM6.")
    
    # Reset the device by toggling DTR/RTS
    print("Resetting device...")
    ser.setDTR(False)
    ser.setRTS(True)
    time.sleep(0.1)
    ser.setRTS(False)
    time.sleep(0.1)
    ser.setDTR(True)
    time.sleep(0.5)
    
    # Flush any leftover data
    ser.reset_input_buffer()
    
    print("Reading boot output for 20 seconds...")
    start = time.time()
    while time.time() - start < 20:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='replace').strip()
            if line:
                print(line)
        else:
            time.sleep(0.01)
    ser.close()
    print("\n--- Done ---")
except Exception as e:
    print(f"Error: {e}")
