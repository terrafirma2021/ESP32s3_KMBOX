# V2.2 Update 

- **Improved Menu Handling**: Implemented state logic for menu management.
- **Simultaneous Menu and Mouse Operation**: Enabled simultaneous combined usage of menu and mouse commands without blocking each other.
- **Mouse Control Parsing**: Now supports HID descriptors to determine the order in the USB transfer buffer for mouse buttons and X, Y.
- **Implemented USB Power Saving**: USB_B_REQUEST_SET_FEATURE Enabled for usb wakeup support, (timeout after 10 seconds).

# TODO:
- **Passive Descriptor Parsing**: Implement passive parsing of HID descriptors to obtain necessary information.
- **WebUI/BLE GATT API for Management**: Develop a WebUI/BLE GATT, API to facilitate device management.

# Known bugs
- **USB Desciptors: partially broken after menu migration to state logic, will solve with next update. 
- ** UART ramdomly Hangs: This happens even with kmbox (Same IDF underpins), temp work around by disabling U1 interupts flushing and re-enabling. I will create a test code and submit a bug report to Expressif asap.


# Bug fixes:
- fixed issue where some mouse may not report 0x81 endpoint correctly. Causing code to block mouse.
- Fixed USB Wireless mouse battery drain.

# ESP32-S3 Dual Dev Kit Project

## Overview

This project utilizes two ESP32-S3 dev kits in a stacked configuration (one on top of the other) to create a versatile USB Host and Device interface. The two dev kits communicate via UART, with specific roles assigned to each:

- **ESP-B (Bottom):** Handles USB Host mode for the mouse via the left USB C connector.
- **ESP-A (Top):** Handles USB Device mode via the left USB C Connector.
- **ESP-A (Top):** replicates KMBOX B Pro commands via the right USB C connector.

## Images

<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/Board1.jpg" width="80%">
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/case.jpg" width="80%">
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/Test_Script_Photo.JPG" width="80%">

## Boards Used

- **YD-ESP32-S3 N16R8**
  - [Amazon](https://www.amazon.co.uk/dp/B0CQNBJSCP)
  - NOTE: *** Please ensure you buy a N16R8 board, i will create a boards.json for you if not, but it will save your time if you get the right device, 

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

## Supported descriptors:
Supported descriptors:

- VID
- PID
- USB Version
- Firmware Version
- Product Name
- Manufacturer Name
- Serial Number
- USB Power
- USB Attributes
- Descriptor4 *Extra*
- Device Descriptor bLength
- Device Descriptor bDescriptorType
- Device Descriptor bcdUSB
- Device Descriptor bDeviceClass
- Device Descriptor bDeviceSubClass
- Device Descriptor bDeviceProtocol
- Device Descriptor bMaxPacketSize0
- Device Descriptor idVendor
- Device Descriptor idProduct
- Device Descriptor bcdDevice
- Device Descriptor iManufacturer
- Device Descriptor iProduct
- Device Descriptor iSerialNumber
- Device Descriptor bNumConfigurations
- Configuration Descriptor bLength
- Configuration Descriptor bDescriptorType
- Configuration Descriptor wTotalLength
- Configuration Descriptor bNumInterfaces
- Configuration Descriptor bConfigurationValue
- Configuration Descriptor iConfiguration
- Configuration Descriptor bmAttributes
- Configuration Descriptor MaxPower
- Interface Descriptor 0 bLength
- Interface Descriptor 0 bDescriptorType
- Interface Descriptor 0 bInterfaceNumber
- Interface Descriptor 0 bAlternateSetting
- Interface Descriptor 0 bNumEndpoints
- Interface Descriptor 0 bInterfaceClass
- Interface Descriptor 0 bInterfaceSubClass
- Interface Descriptor 0 bInterfaceProtocol
- Interface Descriptor 0 iInterface
- HID Descriptor 0 bLength
- HID Descriptor 0 bDescriptorType
- HID Descriptor 0 bcdHID
- HID Descriptor 0 bCountryCode
- HID Descriptor 0 bNumDescriptors
- HID Descriptor 0 bDescriptorType0
- HID Descriptor 0 wDescriptorLength
- Endpoint Descriptor 0 bLength
- Endpoint Descriptor 0 bDescriptorType
- Endpoint Descriptor 0 bEndpointAddress
- Endpoint Descriptor 0 bmAttributes
- Endpoint Descriptor 0 wMaxPacketSize
- Endpoint Descriptor 0 bInterval
- Interface Descriptor 1 bLength
- Interface Descriptor 1 bDescriptorType
- Interface Descriptor 1 bInterfaceNumber
- Interface Descriptor 1 bAlternateSetting
- Interface Descriptor 1 bNumEndpoints
- Interface Descriptor 1 bInterfaceClass
- Interface Descriptor 1 bInterfaceSubClass
- Interface Descriptor 1 bInterfaceProtocol
- Interface Descriptor 1 iInterface
- HID Descriptor 1 bLength
- HID Descriptor 1 bDescriptorType
- HID Descriptor 1 bcdHID
- HID Descriptor 1 bCountryCode
- HID Descriptor 1 bNumDescriptors
- HID Descriptor 1 bDescriptorType0
- HID Descriptor 1 wDescriptorLength
- Endpoint Descriptor 1 bLength
- Endpoint Descriptor 1 bDescriptorType
- Endpoint Descriptor 1 bEndpointAddress
- Endpoint Descriptor 1 bmAttributes
- Endpoint Descriptor 1 wMaxPacketSize
- Endpoint Descriptor 1 bInterval


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

- The time taken to process a command has been benchmarked, showing solid results:
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/Speed.JPG" width="30%">


These results demonstrate the efficiency and speed improvements achieved with the new code.

## Summary

The transition to version 2 of the project has brought significant improvements in efficiency and performance. By utilizing DMA and ISRs for serial communication, the system now handles commands much faster, with average command processing times reduced to under 700 microseconds. These updates have enhanced the overall responsiveness and stability of the project, making it a more robust solution for USB Host and Device interfacing.


## PCB Pinout

Since this is a stacked configuration, all marked pins will be connected directly to both ESP modules. The grounds are shared, so the additional ground connections only enhance board support. Avoid using the 3.3V pin, as we will power the devices via the LDOs.

The use of the IN/OUT pad on the top will provide power to the bottom ESP, ensuring that both ESP modules power up simultaneously when using the top ESP.

Note: This power flow is unidirectional: Top ---> Bottom.
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/pcb_design.png">


## Thanks:
- [thecloneop](https://discord.gg/UqzcRTVpky): Bug tester

## NOTE: 
- When installing python dont forget to add to path!
- Your Version number may differ from photo!
<img src="https://miro.medium.com/v2/resize:fit:1344/0*7nOyowsPsGI19pZT.png">
