# ESP32-S3 Dual Dev Kit Project

## Overview

This project utilizes two ESP32-S3 dev kits in a stacked configuration (one on top of the other) to create a versatile USB Host and Device interface. The two dev kits communicate via UART, with specific roles assigned to each:

- **ESP-B (Bottom):** Handles USB Host mode for the mouse via the left USB C connector.
- **ESP-A (Top):** Handles USB Device mode and replicates KMBOX B Pro commands via the right USB C connector.

## notes:
- ** Buttons, X,Y, wheel  may not match your mouse, enable debug in PIO and adjust the transfer buffer index in the EspUsbHost.cpp (Bottom ESP)

        Void EspUsbHost::_onReceive(usb_transfer_t *transfer) {
  
        report.buttons = transfer->data_buffer[0];
        report.x = (uint8_t)transfer->data_buffer[2];
        report.y = (uint8_t)transfer->data_buffer[4];
        report.wheel = (uint8_t)transfer->data_buffer[6];


## Images


<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/Board1.jpg" width="80%">

<img src="https://github.com/terrafirma2021/ESP32s3_KMBOX/blob/main/Photos/case.jpg" width="80%">


## Boards Used

- **YD-ESP32-S3 N16R8**
  - [Amazon](https://www.amazon.co.uk/dp/B0CQNBJSCP)

## Additional Requirements

- Soldering is required to link the 5V pads (next to the RGB smd) to pass 5V out of the 5V pin on the bottom ESP.
- The USB_OTG pads under the board need to be soldered to enable OTG host mode on the bottom ESP.
- 3D Printer for the case, Its very tight :)  (STL Included), printed on k1max with 0.6 bontech CHT 0 issues

## Connectivity

- A USB-C OTG cable is required to connect a mouse to the ESP.
- Standard USB-C to male USB cables are suitable for COM/USB Device mode.

## Communication Setup

The two ESP32-S3 dev kits communicate using UART, allowing seamless data transfer between them.

## Functionality

- **USB Host Mode (ESP-B):** This dev kit acts as a USB host for the mouse, managing data and sending commands to the other dev kit.
- **USB Device Mode (ESP-A):** This dev kit handles device mode operations and replicates KMBOX B Pro commands through its right USB C connector.

## Files included

- Both Devkit board code
- 3d Printed case
- Py script to test the project

## Menu for Configuring Device Settings

To open the configuration menu, send the command `menu` through the COM port (Top ESP) . The current values will be displayed, and you can change any setting by typing its corresponding number in decimal. To save the changes and reboot the device with new values, type the save number.

**Note:** If all values are removed, the device will revert to default settings to avoid blank values.

### Available Settings

1. **VID:** Vendor ID
2. **PID:** Product ID
3. **USB Version:** USB version information
4. **Firmware Version:** Current firmware version
5. **Product Name:** Name of the product
6. **Manufacturer Name:** Name of the manufacturer
7. **Serial Number:** Serial number of the device
8. **USB Power:** Power requirements of the USB
9. **USB Attributes:** Attributes related to USB configuration
10. **Descriptor4:** Additional descriptor information
14. **Device Descriptor bLength:** Length of the device descriptor
15. **Device Descriptor bDescriptorType:** Type of the device descriptor
16. **Device Descriptor bcdUSB:** USB specification release number
17. **Device Descriptor bDeviceClass:** Class code of the device
18. **Device Descriptor bDeviceSubClass:** Subclass code of the device
19. **Device Descriptor bDeviceProtocol:** Protocol code of the device
20. **Device Descriptor bMaxPacketSize0:** Maximum packet size for endpoint zero
21. **Device Descriptor idVendor:** Vendor ID of the device descriptor
22. **Device Descriptor idProduct:** Product ID of the device descriptor
23. **Device Descriptor bcdDevice:** Device release number
24. **Device Descriptor iManufacturer:** Index of the manufacturer string descriptor
25. **Device Descriptor iProduct:** Index of the product string descriptor
26. **Device Descriptor iSerialNumber:** Index of the serial number string descriptor
27. **Device Descriptor bNumConfigurations:** Number of possible configurations
28. **Configuration Descriptor bLength:** Length of the configuration descriptor
29. **Configuration Descriptor bDescriptorType:** Type of the configuration descriptor
30. **Configuration Descriptor wTotalLength:** Total length of the configuration descriptor
31. **Configuration Descriptor bNumInterfaces:** Number of interfaces supported by this configuration
32. **Configuration Descriptor bConfigurationValue:** Value to use as an argument to select this configuration
33. **Configuration Descriptor iConfiguration:** Index of the configuration string descriptor
34. **Configuration Descriptor bmAttributes:** Configuration characteristics
35. **Configuration Descriptor MaxPower:** Maximum power consumption
36. **Interface Descriptor 0 bLength:** Length of the interface descriptor
37. **Interface Descriptor 0 bDescriptorType:** Type of the interface descriptor
38. **Interface Descriptor 0 bInterfaceNumber:** Number of this interface
39. **Interface Descriptor 0 bAlternateSetting:** Value used to select this alternate setting
40. **Interface Descriptor 0 bNumEndpoints:** Number of endpoints used by this interface
41. **Interface Descriptor 0 bInterfaceClass:** Class code
42. **Interface Descriptor 0 bInterfaceSubClass:** Subclass code
43. **Interface Descriptor 0 bInterfaceProtocol:** Protocol code
44. **Interface Descriptor 0 iInterface:** Index of the string descriptor
45. **HID Descriptor 0 bLength:** Length of the HID descriptor
46. **HID Descriptor 0 bDescriptorType:** Type of the HID descriptor
47. **HID Descriptor 0 bcdHID:** HID Class Specification release
48. **HID Descriptor 0 bCountryCode:** Country code of the hardware target
49. **HID Descriptor 0 bNumDescriptors:** Number of HID class descriptors to follow
50. **HID Descriptor 0 bDescriptorType0:** Type of HID class descriptor
51. **HID Descriptor 0 wDescriptorLength:** Total length of the report descriptor
52. **Endpoint Descriptor 0 bLength:** Length of the endpoint descriptor
53. **Endpoint Descriptor 0 bDescriptorType:** Type of the endpoint descriptor
54. **Endpoint Descriptor 0 bEndpointAddress:** Address of the endpoint
55. **Endpoint Descriptor 0 bmAttributes:** Endpoint attributes
56. **Endpoint Descriptor 0 wMaxPacketSize:** Maximum packet size this endpoint is capable of sending/receiving
57. **Endpoint Descriptor 0 bInterval:** Interval for polling endpoint for data transfers
58. **Interface Descriptor 1 bLength:** Length of the interface descriptor
59. **Interface Descriptor 1 bDescriptorType:** Type of the interface descriptor
60. **Interface Descriptor 1 bInterfaceNumber:** Number of this interface
61. **Interface Descriptor 1 bAlternateSetting:** Value used to select this alternate setting
62. **Interface Descriptor 1 bNumEndpoints:** Number of endpoints used by this interface
63. **Interface Descriptor 1 bInterfaceClass:** Class code
64. **Interface Descriptor 1 bInterfaceSubClass:** Subclass code
65. **Interface Descriptor 1 bInterfaceProtocol:** Protocol code
66. **Interface Descriptor 1 iInterface:** Index of the string descriptor
67. **HID Descriptor 1 bLength:** Length of the HID descriptor
68. **HID Descriptor 1 bDescriptorType:** Type of the HID descriptor
69. **HID Descriptor 1 bcdHID:** HID Class Specification release
70. **HID Descriptor 1 bCountryCode:** Country code of the hardware target
71. **HID Descriptor 1 bNumDescriptors:** Number of HID class descriptors to follow
72. **HID Descriptor 1 bDescriptorType0:** Type of HID class descriptor
73. **HID Descriptor 1 wDescriptorLength:** Total length of the report descriptor
74. **Endpoint Descriptor 1 bLength:** Length of the endpoint descriptor
75. **Endpoint Descriptor 1 bDescriptorType:** Type of the endpoint descriptor
76. **Endpoint Descriptor 1 bEndpointAddress:** Address of the endpoint
77. **Endpoint Descriptor 1 bmAttributes:** Endpoint attributes
78. **Endpoint Descriptor 1 wMaxPacketSize:** Maximum packet size this endpoint is capable of sending/receiving
79. **Endpoint Descriptor 1 bInterval:** Interval for polling endpoint for data transfers
80. **Print Current Values**
81. **Delete Values Stored**
82. **Exit and Save**

Please be sure to change the values to your own, as the provided default values will only be available for a limited time.

Enjoy!
