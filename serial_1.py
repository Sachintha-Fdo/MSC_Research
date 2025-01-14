import serial
import time
import csv

def read_arduino_and_save_to_csv():
    try:
        # Replace 'COM8' with your Arduino's COM port
        port_name = "COM8"
        baud_rate = 9600
        timeout = 2  # Timeout in seconds
        output_csv = "sensor_data.csv"  # File to save data

        # Open the serial connection
        print(f"Connecting to {port_name} at {baud_rate} baud...")
        ser = serial.Serial(port=port_name, baudrate=baud_rate, timeout=timeout)
        print(f"Connected to {ser.name}")

        # Allow Arduino to reset
        time.sleep(2)
        ser.flush()

        # Open the CSV file for writing
        with open(output_csv, mode="w", newline="") as csv_file:
            csv_writer = csv.writer(csv_file)
            # Write CSV header
            csv_writer.writerow([
                "Time (ms)",
                "Temperature (°C)",
                "Humidity (%)",
                "CO2 (ppm)",
                "Ethylene (ppm)",
                "Ammonia (ppm)",
                "Red",
                "Green",
                "Blue",
                "Clear",
                "Color Temp (K)",
                "Absorption",
                "Emission"
            ])
            print(f"Saving data to {output_csv}...")

            # Read and process data
            while True:
                if ser.in_waiting > 0:
                    data = ser.readline().decode("utf-8").strip()
                    print(f"Raw data: {data}")

                    # Parse CSV data
                    try:
                        fields = data.split(",")
                        if len(fields) == 13:  # Ensure all expected fields are present
                            csv_writer.writerow(fields)  # Save to CSV
                            csv_file.flush()  # Ensure data is written to file
                            print("Data saved:", fields)
                        else:
                            print(f"Unexpected number of fields: {len(fields)}")
                    except Exception as e:
                        print(f"Failed to parse data: {e}")

    except serial.SerialException as e:
        print(f"SerialException: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    finally:
        # Close the serial connection on exit
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial port closed.")

if __name__ == "__main__":
    read_arduino_and_save_to_csv()
