#pragma once

#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "SPIFFS.h"
#include <KMBOX.h>
#include <menu.h>
#include <spiffsconf.h>

extern USBHIDMouse Mouse;

// Default USB settings
String vid = "0x046D";                              // Vendor ID
String pid = "0xC09D";                              // Product ID
String usbVersion = "0x0200";                       // USB Version 2.0
String firmwareVersion = "0x5202";                  // Device firmware version
String productName = "G203 LIGHTSYNC Gaming Mouse"; // Product name
String manufacturerName = "Logitech";               // Manufacturer name
String serialNumber = "11A33D5C3A24";               // Serial number
String usbPower = "300";                            // Maximum power in mA
String usbAttributes = "0xA0";                      // Configuration attributes
uint16_t languageID = 0x0409;                       // Language ID as a variable
String descriptor4 = "U152.02_B0017           ";    // Software Version??

// Device Descriptor variables
uint8_t deviceDescriptor_bLength = 0x12;
uint8_t deviceDescriptor_bDescriptorType = 0x01;
uint16_t deviceDescriptor_bcdUSB = 0x0200;
uint8_t deviceDescriptor_bDeviceClass = 0x00;
uint8_t deviceDescriptor_bDeviceSubClass = 0x00;
uint8_t deviceDescriptor_bDeviceProtocol = 0x00;
uint8_t deviceDescriptor_bMaxPacketSize0 = 0x40;
uint16_t deviceDescriptor_idVendor = 0x046D;
uint16_t deviceDescriptor_idProduct = 0xC09D;
uint16_t deviceDescriptor_bcdDevice = 0x5202;
uint8_t deviceDescriptor_iManufacturer = 0x01;
uint8_t deviceDescriptor_iProduct = 0x02;
uint8_t deviceDescriptor_iSerialNumber = 0x03;
uint8_t deviceDescriptor_bNumConfigurations = 0x01;

// Configuration Descriptor variables
uint8_t configurationDescriptor_bLength = 0x09;
uint8_t configurationDescriptor_bDescriptorType = 0x02;
uint16_t configurationDescriptor_wTotalLength = 0x003B;
uint8_t configurationDescriptor_bNumInterfaces = 0x02;
uint8_t configurationDescriptor_bConfigurationValue = 0x01;
uint8_t configurationDescriptor_iConfiguration = 0x04;
uint8_t configurationDescriptor_bmAttributes = 0xA0;
uint8_t configurationDescriptor_MaxPower = 0x96; // 300 mA

// Interface Descriptor 0 variables
uint8_t interfaceDescriptor0_bLength = 0x09;
uint8_t interfaceDescriptor0_bDescriptorType = 0x04;
uint8_t interfaceDescriptor0_bInterfaceNumber = 0x00;
uint8_t interfaceDescriptor0_bAlternateSetting = 0x00;
uint8_t interfaceDescriptor0_bNumEndpoints = 0x01;
uint8_t interfaceDescriptor0_bInterfaceClass = 0x03;
uint8_t interfaceDescriptor0_bInterfaceSubClass = 0x01;
uint8_t interfaceDescriptor0_bInterfaceProtocol = 0x02;
uint8_t interfaceDescriptor0_iInterface = 0x00;

// HID Descriptor 0 variables
uint8_t hidDescriptor0_bLength = 0x09;
uint8_t hidDescriptor0_bDescriptorType = 0x21;
uint16_t hidDescriptor0_bcdHID = 0x0111;
uint8_t hidDescriptor0_bCountryCode = 0x00;
uint8_t hidDescriptor0_bNumDescriptors = 0x01;
uint8_t hidDescriptor0_bDescriptorType0 = 0x22;
uint16_t hidDescriptor0_wDescriptorLength = 0x0043;

// Endpoint Descriptor 0 variables
uint8_t endpointDescriptor0_bLength = 0x07;
uint8_t endpointDescriptor0_bDescriptorType = 0x05;
uint8_t endpointDescriptor0_bEndpointAddress = 0x81;
uint8_t endpointDescriptor0_bmAttributes = 0x03;
uint16_t endpointDescriptor0_wMaxPacketSize = 0x0008;
uint8_t endpointDescriptor0_bInterval = 0x01;

// Interface Descriptor 1 variables
uint8_t interfaceDescriptor1_bLength = 0x09;
uint8_t interfaceDescriptor1_bDescriptorType = 0x04;
uint8_t interfaceDescriptor1_bInterfaceNumber = 0x01;
uint8_t interfaceDescriptor1_bAlternateSetting = 0x00;
uint8_t interfaceDescriptor1_bNumEndpoints = 0x01;
uint8_t interfaceDescriptor1_bInterfaceClass = 0x03;
uint8_t interfaceDescriptor1_bInterfaceSubClass = 0x00;
uint8_t interfaceDescriptor1_bInterfaceProtocol = 0x00;
uint8_t interfaceDescriptor1_iInterface = 0x00;

// HID Descriptor 1 variables
uint8_t hidDescriptor1_bLength = 0x09;
uint8_t hidDescriptor1_bDescriptorType = 0x21;
uint16_t hidDescriptor1_bcdHID = 0x0111;
uint8_t hidDescriptor1_bCountryCode = 0x00;
uint8_t hidDescriptor1_bNumDescriptors = 0x01;
uint8_t hidDescriptor1_bDescriptorType0 = 0x22;
uint16_t hidDescriptor1_wDescriptorLength = 0x0097;

// Endpoint Descriptor 1 variables
uint8_t endpointDescriptor1_bLength = 0x07;
uint8_t endpointDescriptor1_bDescriptorType = 0x05;
uint8_t endpointDescriptor1_bEndpointAddress = 0x82;
uint8_t endpointDescriptor1_bmAttributes = 0x03;
uint16_t endpointDescriptor1_wMaxPacketSize = 0x0014;
uint8_t endpointDescriptor1_bInterval = 0x01;

// Arrays combining the descriptors
uint8_t const deviceDescriptor[] = {
    deviceDescriptor_bLength,
    deviceDescriptor_bDescriptorType,
    uint8_t(deviceDescriptor_bcdUSB & 0xFF),
    uint8_t((deviceDescriptor_bcdUSB >> 8) & 0xFF),
    deviceDescriptor_bDeviceClass,
    deviceDescriptor_bDeviceSubClass,
    deviceDescriptor_bDeviceProtocol,
    deviceDescriptor_bMaxPacketSize0,
    uint8_t(deviceDescriptor_idVendor & 0xFF),
    uint8_t((deviceDescriptor_idVendor >> 8) & 0xFF),
    uint8_t(deviceDescriptor_idProduct & 0xFF),
    uint8_t((deviceDescriptor_idProduct >> 8) & 0xFF),
    uint8_t(deviceDescriptor_bcdDevice & 0xFF),
    uint8_t((deviceDescriptor_bcdDevice >> 8) & 0xFF),
    deviceDescriptor_iManufacturer,
    deviceDescriptor_iProduct,
    deviceDescriptor_iSerialNumber,
    deviceDescriptor_bNumConfigurations};

uint8_t const configurationDescriptor[] = {

    // Configuration Descriptor
    configurationDescriptor_bLength,
    configurationDescriptor_bDescriptorType,
    uint8_t(configurationDescriptor_wTotalLength & 0xFF),
    uint8_t((configurationDescriptor_wTotalLength >> 8) & 0xFF),
    configurationDescriptor_bNumInterfaces,
    configurationDescriptor_bConfigurationValue,
    configurationDescriptor_iConfiguration,
    configurationDescriptor_bmAttributes,
    configurationDescriptor_MaxPower,

    // Interface Descriptor 0
    interfaceDescriptor0_bLength,
    interfaceDescriptor0_bDescriptorType,
    interfaceDescriptor0_bInterfaceNumber,
    interfaceDescriptor0_bAlternateSetting,
    interfaceDescriptor0_bNumEndpoints,
    interfaceDescriptor0_bInterfaceClass,
    interfaceDescriptor0_bInterfaceSubClass,
    interfaceDescriptor0_bInterfaceProtocol,
    interfaceDescriptor0_iInterface,

    // HID Descriptor 0
    hidDescriptor0_bLength,
    hidDescriptor0_bDescriptorType,
    uint8_t(hidDescriptor0_bcdHID & 0xFF),
    uint8_t((hidDescriptor0_bcdHID >> 8) & 0xFF),
    hidDescriptor0_bCountryCode,
    hidDescriptor0_bNumDescriptors,
    hidDescriptor0_bDescriptorType0,
    uint8_t(hidDescriptor0_wDescriptorLength & 0xFF),
    uint8_t((hidDescriptor0_wDescriptorLength >> 8) & 0xFF),

    // Endpoint Descriptor 0
    endpointDescriptor0_bLength,
    endpointDescriptor0_bDescriptorType,
    endpointDescriptor0_bEndpointAddress,
    endpointDescriptor0_bmAttributes,
    uint8_t(endpointDescriptor0_wMaxPacketSize & 0xFF),
    uint8_t((endpointDescriptor0_wMaxPacketSize >> 8) & 0xFF),
    endpointDescriptor0_bInterval,

    // Interface Descriptor 1
    interfaceDescriptor1_bLength,
    interfaceDescriptor1_bDescriptorType,
    interfaceDescriptor1_bInterfaceNumber,
    interfaceDescriptor1_bAlternateSetting,
    interfaceDescriptor1_bNumEndpoints,
    interfaceDescriptor1_bInterfaceClass,
    interfaceDescriptor1_bInterfaceSubClass,
    interfaceDescriptor1_bInterfaceProtocol,
    interfaceDescriptor1_iInterface,

    // HID Descriptor 1
    hidDescriptor1_bLength,
    hidDescriptor1_bDescriptorType,
    uint8_t(hidDescriptor1_bcdHID & 0xFF),
    uint8_t((hidDescriptor1_bcdHID >> 8) & 0xFF),
    hidDescriptor1_bCountryCode,
    hidDescriptor1_bNumDescriptors,
    hidDescriptor1_bDescriptorType0,
    uint8_t(hidDescriptor1_wDescriptorLength & 0xFF),
    uint8_t((hidDescriptor1_wDescriptorLength >> 8) & 0xFF),

    // Endpoint Descriptor 1
    endpointDescriptor1_bLength,
    endpointDescriptor1_bDescriptorType,
    endpointDescriptor1_bEndpointAddress,
    endpointDescriptor1_bmAttributes,
    uint8_t(endpointDescriptor1_wMaxPacketSize & 0xFF),
    uint8_t((endpointDescriptor1_wMaxPacketSize >> 8) & 0xFF),
    endpointDescriptor1_bInterval};

static uint16_t _desc_str[32 + 1];

// String Descriptor Callback
extern "C" uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    size_t chr_count = 0;
    const char *str = NULL;
    size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;

    switch (index)
    {
    case 0:
        _desc_str[1] = languageID;
        chr_count = 1;
        break;
    case 1:
        str = manufacturerName.c_str();
        break;
    case 2:
        str = productName.c_str();
        break;
    case 3:
        str = serialNumber.c_str();
        break;
    case 4:
        str = descriptor4.c_str();
        break;
    default:
        return NULL;
    }

    if (str)
    {
        chr_count = strlen(str);
        if (chr_count > max_count)
            chr_count = max_count;
        for (size_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}

// Device Descriptor Callback
extern "C" uint8_t const *tud_descriptor_device_cb(void)
{
    return deviceDescriptor;
}

void InitUSB()
{
    USB.VID(strtoul(vid.c_str(), NULL, 16));
    USB.PID(strtoul(pid.c_str(), NULL, 16));
    USB.usbVersion(strtoul(usbVersion.c_str(), NULL, 16));
    USB.firmwareVersion(strtoul(firmwareVersion.c_str(), NULL, 16));
    USB.productName(productName.c_str());
    USB.manufacturerName(manufacturerName.c_str());
    USB.serialNumber(serialNumber.c_str());
    USB.usbPower(usbPower.toInt());
    USB.usbAttributes(strtoul(usbAttributes.c_str(), NULL, 16));
    Mouse.begin();
    USB.begin();
}
