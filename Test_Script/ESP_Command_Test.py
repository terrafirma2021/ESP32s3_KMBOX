import serial
import serial.tools.list_ports
import threading
import time
import keyboard

# Constants for command mappings
MOVE_COMMAND = "km.move"
MOVETO_COMMAND = "km.moveto"
GETPOS_COMMAND = "km.getpos"
BUTTON1_DOWN_COMMAND = "km.left(1)"
BUTTON1_UP_COMMAND = "km.left(0)"
BUTTON2_DOWN_COMMAND = "km.right(1)"
BUTTON2_UP_COMMAND = "km.right(0)"
BUTTON3_DOWN_COMMAND = "km.middle(1)"
BUTTON3_UP_COMMAND = "km.middle(0)"
BUTTON4_DOWN_COMMAND = "km.side1(1)"
BUTTON4_UP_COMMAND = "km.side1(0)"
BUTTON5_DOWN_COMMAND = "km.side2(1)"
BUTTON5_UP_COMMAND = "km.side2(0)"
WHEEL_COMMAND = "km.wheel"

# Function to list all available COM ports
def list_com_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

# Function to select COM port
def select_com_port():
    ports = list_com_ports()
    if not ports:
        print("No COM ports found.")
        return None
    for i, port in enumerate(ports):
        print(f"{i}: {port}")
    port_index = int(input("Select COM port by number: "))
    return ports[port_index]

# Function to send command to the selected COM port
def send_command(ser, command):
    command_to_send = f"{command}\n"
    ser.write(command_to_send.encode())
    print(f"Sent: {command_to_send}", end='\r')

# Function to read data from the serial port
def read_from_port(ser):
    while True:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').rstrip()
                print(f"Received: {line}")
        except serial.SerialException as e:
            print(f"Error reading from serial port: {e}")
            break

# Main function
def main():
    print("Listing available COM ports...")
    com_port = select_com_port()
    if com_port is None:
        print("No COM port selected, exiting.")
        return

    print(f"Opening COM port: {com_port} at 115200 baud rate")
    try:
        ser = serial.Serial(com_port, 115200, timeout=1)
    except serial.SerialException as e:
        print(f"Failed to open COM port: {e}")
        return
    print(f"COM port {com_port} opened.")

    try:
        # Start reading from the serial port in a separate thread
        print("Starting to read from the serial port in a separate thread.")
        read_thread = threading.Thread(target=read_from_port, args=(ser,))
        read_thread.daemon = True
        read_thread.start()

        print("""
        +---------------------------------------+
        | Use the following keys to control the |
        | mouse:                                |
        |                                       |
        | Movement:                             |
        |   W - Move Up                         |
        |   S - Move Down                       |
        |   A - Move Left                       |
        |   D - Move Right                      |
        |                                       |
        | Buttons:                              |
        |   Z - Left Click                      |
        |   X - Right Click                     |
        |   C - Middle Click                    |
        |   V - Button 4                        |
        |   B - Button 5                        |
        |                                       |
        | Wheel:                                |
        |   N - Wheel Up                        |
        |   M - Wheel Down                      |
        +---------------------------------------+
        """)

        def on_event(event):
            if event.event_type == 'down':
                if event.name == 'w':
                    send_command(ser, f"{MOVE_COMMAND}(0,-10)")
                elif event.name == 's':
                    send_command(ser, f"{MOVE_COMMAND}(0,10)")
                elif event.name == 'a':
                    send_command(ser, f"{MOVE_COMMAND}(-10,0)")
                elif event.name == 'd':
                    send_command(ser, f"{MOVE_COMMAND}(10,0)")
                elif event.name == 'z':
                    send_command(ser, BUTTON1_DOWN_COMMAND)
                elif event.name == 'x':
                    send_command(ser, BUTTON2_DOWN_COMMAND)
                elif event.name == 'c':
                    send_command(ser, BUTTON3_DOWN_COMMAND)
                elif event.name == 'v':
                    send_command(ser, BUTTON4_DOWN_COMMAND)
                elif event.name == 'b':
                    send_command(ser, BUTTON5_DOWN_COMMAND)
                elif event.name == 'n':
                    send_command(ser, f"{WHEEL_COMMAND}(1)")
                elif event.name == 'm':
                    send_command(ser, f"{WHEEL_COMMAND}(-1)")
                print(f"Key pressed: {event.name}")  # Debug statement
            elif event.event_type == 'up':
                if event.name == 'z':
                    send_command(ser, BUTTON1_UP_COMMAND)
                elif event.name == 'x':
                    send_command(ser, BUTTON2_UP_COMMAND)
                elif event.name == 'c':
                    send_command(ser, BUTTON3_UP_COMMAND)
                elif event.name == 'v':
                    send_command(ser, BUTTON4_UP_COMMAND)
                elif event.name == 'b':
                    send_command(ser, BUTTON5_UP_COMMAND)
                print(f"Key released: {event.name}")  # Debug statement

        # Set up key event listener
        keyboard.hook(on_event)

        # Keep the main thread alive
        while True:
            time.sleep(0.01)

    except KeyboardInterrupt:
        print("KeyboardInterrupt received, exiting.")
    finally:
        print(f"Closing COM port {com_port}.")
        ser.close()

if __name__ == "__main__":
    main()
