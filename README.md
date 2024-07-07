# ESP32-S3 Dual Dev Kit Project

## Overview

This project utilizes two ESP32-S3 dev kits in a stacked configuration (one on top of the other) to create a versatile USB Host and Device interface. The two dev kits communicate via UART, with specific roles assigned to each:

- **ESP-B (Bottom):** Handles USB Host mode for the mouse via the left USB C connector.
- **ESP-A (Top):** Handles USB Device mode via the left USB C Connector.
- **ESP-A (Top):** replicates KMBOX B Pro commands via the right USB C connector.

## Notes:
- **Buttons, X, Y, wheel may not match your mouse, enable debug in PIO and adjust the transfer buffer index in the EspUsbHost.cpp (Bottom ESP)**

    ```cpp
    Void EspUsbHost::_onReceive(usb_transfer_t *transfer) {
        report.buttons = transfer->data_buffer[0];
        report.x = (uint8_t)transfer->data_buffer[2];
        report.y = (uint8_t)transfer->data_buffer[4];
        report.wheel = (uint8_t)transfer->data_buffer[6];
    }
    ```

## Images

<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/Board1.jpg" width="80%">
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/case.jpg" width="80%">
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/Test_Script_Photo.JPG" width="80%">

## Boards Used

- **YD-ESP32-S3 N16R8**
  - [Amazon](https://www.amazon.co.uk/dp/B0CQNBJSCP)

## Additional Requirements

- Soldering is required to link the 5V pads (next to the RGB smd) to pass 5V out of the 5V pin on the bottom ESP.
- The USB_OTG pads under the board need to be soldered to enable OTG host mode on the bottom ESP.
- 3D Printer for the case, Its very tight :) (STL Included), printed on k1max with 0.6 bontech CHT 0 issues.

## Connectivity

- A USB-C OTG cable is required to connect a mouse to the ESP.
- Standard USB-C to male USB cables are suitable for COM/USB Device mode.

## Communication Setup

The two ESP32-S3 dev kits communicate using UART, allowing seamless data transfer between them.

## Functionality

- **USB Host Mode (ESP-B):** This dev kit acts as a USB host for the mouse, managing data and sending commands to the other dev kit.
- **USB Device Mode (ESP-A):** This dev kit handles device mode operations and replicates KMBOX B Pro commands through its right USB C connector.

## Files Included

- Both Devkit board code
- 3D Printed case
- Python script to test the project

## Menu for Configuring Device Settings

To open the configuration menu, send the command `menu` through the COM port (Bottom ESP). The current values will be displayed, and you can change any setting by typing its corresponding number in decimal. To save the changes and reboot the device with new values, type the save number.

**Note:** If all values are removed, the device will revert to default settings to avoid blank values.

## Updates and Improvements in V2

### Enhanced Efficiency and Speed

- **DMA Implementation:** The updated code utilizes DMA (Direct Memory Access) for UART data reception. This significantly reduces CPU overhead and increases the efficiency of data handling.
- **Interrupt Service Routines (ISR):** DMA interrupts are used to handle incoming data, which allows for non-blocking serial communication and reduces latency.
- **Improved Command Processing with DMA Buffers:** The code now leverages DMA buffers for command processing. This enhances efficiency compared to the standard Arduino method of using regular buffers by offloading memory access operations from the CPU to the DMA controller, reducing the average time to process each command.

### Performance Metrics

- **Polling Rate:** Increased from 500Hz to 1000Hz, doubling the frequency at which the device polls for data and improving responsiveness.
- **Command Processing Time:** The total time to process each command, from the ISR to the `Mouse.move` command, is now consistently under 700 microseconds on average. This is a significant improvement over the previous implementation.
- **Stable 4Mbps UART Link:** The communication between the two ESP32-S3 dev kits is now established with a stable 4Mbps UART link, enhancing data transfer rates and reducing latency.

### Stability and Error Handling

- **Graceful Handling of Unknown Commands:** The updated code includes detailed error messages for unknown commands and potential issues, ensuring that the system remains stable and providing useful feedback for debugging.

### Example Timing Improvements

- The time taken to process a command has been benchmarked, showing results such as:
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/Board1.jpg" width="30%">

These results demonstrate the efficiency and speed improvements achieved with the new code.

## Summary

The transition to version 2 of the project has brought significant improvements in efficiency and performance. By utilizing DMA and ISRs for serial communication, the system now handles commands much faster, with average command processing times reduced to under 700 microseconds. These updates have enhanced the overall responsiveness and stability of the project, making it a more robust solution for USB Host and Device interfacing.
