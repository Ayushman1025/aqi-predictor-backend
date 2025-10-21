import serial
import time
import requests

ser = serial.Serial('COM5', 9600, timeout=1)
time.sleep(2)
print("Serial connection established.")

current_data = {}

try:
    while True:
        line = ser.readline().decode('utf-8').strip()

        if not line or "-----" in line or "----" in line:
            continue

        try:
            key, value = line.split(":", 1)
            value = value.strip().split(" ")[0]
            value = float(value)
            key = key.upper()

            if key in ["NO", "NO2", "NOX", "NH3", "CO", "BENZENE", "TOLUENE", "XYLENE", "AQI"]:
                current_data[key] = value

            if len(current_data) == 9:
                url = "https://server.aimliedc.tech/aqi-predictor/update" 
                try:
                    response = requests.post(url, json=current_data)
                    print("Data sent to server:", response.json())
                except Exception as net_err:
                    print("Failed to send data to server:", net_err)

                current_data.clear()

        except ValueError:
            pass

except KeyboardInterrupt:
    print("\n--- Data Collection Stopped ---")
    ser.close()
