# Project has now been merged with the MAKCM project

All code updates will now be found on my repo: [MAKCM GitHub Repository](https://github.com/terrafirma2021/MAKCM).

Names for people who are using MAKCM bins:

Top = LEFT_*.bin (DEVICE)

Bottom = RIGHT_*.bin (HOST)




# ESP32-S3 Dual Dev Kit Project

## Overview

This project utilizes two ESP32-S3 dev kits in a stacked configuration (one on top of the other) to create a versatile USB Host and Device interface. The two dev kits communicate via UART, with specific roles assigned to each:

- **ESP-B (Bottom):** Handles USB Host mode for the mouse via the left USB C connector.
- **ESP-A (Top):** Handles USB Device mode via the left USB C Connector.
- **ESP-A (Top):** replicates KMBOX B Pro commands via the right USB C connector.

## Images
Finished Design:

<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/New_Photos/TOP_Done.jpg" width="80%">




Bottom ESP Soldered pins (USB-OTG pad must be soldered!) :

<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/New_Photos/Bottom2.jpg" width="80%">



Top ESP Soldered pins:

<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/New_Photos/Top1.jpg" width="80%">


Side view of soldered pins:

<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/New_Photos/Side1.jpg" width="80%">


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




## Summary

The transition to version 2 of the project has brought significant improvements in efficiency and performance. By utilizing DMA and ISRs for serial communication, the system now handles commands much faster, with average command processing times reduced to under 700 microseconds. These updates have enhanced the overall responsiveness and stability of the project, making it a more robust solution for USB Host and Device interfacing.


## PCB Pinout

Since this is a stacked configuration, all marked pins will be connected directly to both ESP modules. The grounds are shared, so the additional ground connections only enhance board support. Avoid using the 3.3V pin, as we will power the devices via the LDOs.

The use of the IN/OUT pad on the top will provide power to the bottom ESP, ensuring that both ESP modules power up simultaneously when using the top ESP.

Note: This power flow is -Not Unidirectional-! its direction of VCC is : Top ---> Bottom.
<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/New_Photos/Diagram.png">


## Thanks:
- [thecloneop](https://discord.gg/UqzcRTVpky): Bug tester
- Many more, will add you all when i get onto the list;

## NOTE: 
- When installing python dont forget to add to path!
- Your Version number may differ from photo!
<img src="https://miro.medium.com/v2/resize:fit:1344/0*7nOyowsPsGI19pZT.png">
