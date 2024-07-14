import os
import sys
import subprocess
import threading

# Ensure pyserial is installed
try:
    import serial
except ImportError:
    subprocess.check_call([sys.executable, "-m", "pip", "install", "pyserial"])
    import serial

# Function to get the list of available COM ports
def get_com_ports():
    import serial.tools.list_ports
    return list(serial.tools.list_ports.comports())

# Function to write data to the log file
def write_to_log(data):
    file_path = os.path.join(os.path.dirname(__file__), "log.txt")
    with open(file_path, "a") as log_file:
        log_file.write(data + "\n")

# Function to read data from the serial port
def read_from_serial(ser):
    while True:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    write_to_log(line)
                    print("Received:", line)
            except Exception as e:
                print(f"Error reading from serial port: {e}")

# Function to write data to the serial port
def write_to_serial(ser):
    while True:
        user_input = input("Enter command: ")
        if user_input:
            try:
                ser.write(user_input.encode('utf-8') + b'\n')
            except Exception as e:
                print(f"Error writing to serial port: {e}")

# Main function to setup serial connection and start threads
def main():
    ports = get_com_ports()
    if not ports:
        raise Exception("No COM ports found")

    baud_rates = [115200, 230400, 345600, 460800, 4000000]

    print("Available COM ports:")
    for i, port in enumerate(ports):
        for j, baud_rate in enumerate(baud_rates):
            print(f"{i * len(baud_rates) + j}: {port.device} ({baud_rate})")

    port_index = int(input("Select COM port by number: "))
    
    selected_port = port_index // len(baud_rates)
    selected_baud_rate = baud_rates[port_index % len(baud_rates)]
    com_port = ports[selected_port].device

    with serial.Serial(com_port, selected_baud_rate, timeout=0) as ser:
        print(f"Connected to {com_port} with baud rate {selected_baud_rate}")
        
        read_thread = threading.Thread(target=read_from_serial, args=(ser,))
        write_thread = threading.Thread(target=write_to_serial, args=(ser,))
        
        read_thread.start()
        write_thread.start()
        
        read_thread.join()
        write_thread.join()

if __name__ == "__main__":
    main()
