import serial
import matplotlib.pyplot as plt
from collections import deque

# --- CONFIGURATION ---
SERIAL_PORT = 'COM3' # This looks correct based on your screenshot
BAUD_RATE = 9600
MAX_DATA_POINTS = 50
# ---------------------

print(f"Connecting to {SERIAL_PORT} at {BAUD_RATE} baud...")
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"Error: Could not open port {SERIAL_PORT}.")
    print(f"Details: {e}")
    exit()

print("Connection successful. Reading data...")

data = deque(maxlen=MAX_DATA_POINTS)
plt.ion()
fig, ax = plt.subplots()

try:
    while True:
        try:
            # Read and decode the line
            line = ser.readline().decode('utf-8').strip()

            if line:  # If the line is not empty
                # Convert to a number
                distance = float(line)
                
                # Add to our data
                data.append(distance)
                
                # --- Plotting ---
                ax.clear()
                ax.plot(data)
                ax.set_title("Live Ultrasonic Sensor Data")
                ax.set_ylabel("Distance (cm)")
                ax.set_xlabel("Time (samples)")
                # ax.set_ylim(0, 50) # Uncomment this to lock the y-axis
                plt.pause(0.01)

        # --- UPDATED ERROR HANDLING ---
        except (UnicodeDecodeError):
            # This happens if baud rates are wrong or line is noisy
            print("Ignoring garbage data (check baud rate)...")
        except (ValueError):
            # This happens if it reads "start" or other text
            print(f"Ignoring non-numeric data: '{line}'")
        # -------------------------------
        
        except KeyboardInterrupt:
            print("Stopping and closing serial port.")
            break

finally:
    ser.close()
    plt.ioff()
    print("Serial port closed.")