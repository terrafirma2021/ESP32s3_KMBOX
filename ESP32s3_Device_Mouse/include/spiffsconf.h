#pragma once
#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "SPIFFS.h"
#include <KMBOX.h>
#include <menu.h>
#include <spiffsconf.h>
#include <USBSetup.h>

// Function prototypes
void loadConfig();
void saveToSpiffs();
void InitSpiffs();
void printConfigFile();
extern void deviceMenu(const char *command);
void deleteConfig();

const char *configFilePath = "/config.txt";

void InitSpiffs()
{
    if (!SPIFFS.begin(true))
    {
        Serial0.println("SPIFFS Mount Failed");
        return;
    }

    if (!SPIFFS.exists(configFilePath))
    {
        // Serial0.println("Config file not found, creating with default values..."); Debug Dun
        saveToSpiffs();
    }
    else
    {
        // Serial0.println("Config file found, loading values..."); Debug Dun
        loadConfig();
    }
}

void loadConfig()
{
    File configFile = SPIFFS.open(configFilePath, FILE_READ);
    if (!configFile)
    {
        Serial0.println("Failed to open config file");
        return;
    }
    if (configFile.available())
    {
        while (configFile.available())
        {
            String line = configFile.readStringUntil('\n');
            line.trim();
            int separatorIndex = line.indexOf('=');
            if (separatorIndex != -1)
            {
                String key = line.substring(0, separatorIndex);
                String value = line.substring(separatorIndex + 1);

                // Device Information
                if (key == "VID")
                    vid = value;
                else if (key == "PID")
                    pid = value;
                else if (key == "USBVersion")
                    usbVersion = value;
                else if (key == "FirmwareVersion")
                    firmwareVersion = value;
                else if (key == "ProductName")
                    productName = value;
                else if (key == "ManufacturerName")
                    manufacturerName = value;
                else if (key == "SerialNumber")
                    serialNumber = value;
                else if (key == "USBPower")
                    usbPower = value;
                else if (key == "USBAttributes")
                    usbAttributes = value;

                // Device Descriptor
                else if (key == "DeviceDescriptor_bLength")
                    deviceDescriptor_bLength = value.toInt();
                else if (key == "DeviceDescriptor_bDescriptorType")
                    deviceDescriptor_bDescriptorType = value.toInt();
                else if (key == "DeviceDescriptor_bcdUSB")
                    deviceDescriptor_bcdUSB = strtoul(value.c_str(), NULL, 16);
                else if (key == "DeviceDescriptor_bDeviceClass")
                    deviceDescriptor_bDeviceClass = value.toInt();
                else if (key == "DeviceDescriptor_bDeviceSubClass")
                    deviceDescriptor_bDeviceSubClass = value.toInt();
                else if (key == "DeviceDescriptor_bDeviceProtocol")
                    deviceDescriptor_bDeviceProtocol = value.toInt();
                else if (key == "DeviceDescriptor_bMaxPacketSize0")
                    deviceDescriptor_bMaxPacketSize0 = value.toInt();
                else if (key == "DeviceDescriptor_idVendor")
                    deviceDescriptor_idVendor = strtoul(value.c_str(), NULL, 16);
                else if (key == "DeviceDescriptor_idProduct")
                    deviceDescriptor_idProduct = strtoul(value.c_str(), NULL, 16);
                else if (key == "DeviceDescriptor_bcdDevice")
                    deviceDescriptor_bcdDevice = strtoul(value.c_str(), NULL, 16);
                else if (key == "DeviceDescriptor_iManufacturer")
                    deviceDescriptor_iManufacturer = value.toInt();
                else if (key == "DeviceDescriptor_iProduct")
                    deviceDescriptor_iProduct = value.toInt();
                else if (key == "DeviceDescriptor_iSerialNumber")
                    deviceDescriptor_iSerialNumber = value.toInt();
                else if (key == "DeviceDescriptor_bNumConfigurations")
                    deviceDescriptor_bNumConfigurations = value.toInt();

                // Configuration Descriptor
                else if (key == "ConfigurationDescriptor_bLength")
                    configurationDescriptor_bLength = value.toInt();
                else if (key == "ConfigurationDescriptor_bDescriptorType")
                    configurationDescriptor_bDescriptorType = value.toInt();
                else if (key == "ConfigurationDescriptor_wTotalLength")
                    configurationDescriptor_wTotalLength = strtoul(value.c_str(), NULL, 16);
                else if (key == "ConfigurationDescriptor_bNumInterfaces")
                    configurationDescriptor_bNumInterfaces = value.toInt();
                else if (key == "ConfigurationDescriptor_bConfigurationValue")
                    configurationDescriptor_bConfigurationValue = value.toInt();
                else if (key == "ConfigurationDescriptor_iConfiguration")
                    configurationDescriptor_iConfiguration = value.toInt();
                else if (key == "ConfigurationDescriptor_bmAttributes")
                    configurationDescriptor_bmAttributes = value.toInt();
                else if (key == "ConfigurationDescriptor_MaxPower")
                    configurationDescriptor_MaxPower = value.toInt();

                // Interface Descriptor 0
                else if (key == "InterfaceDescriptor0_bLength")
                    interfaceDescriptor0_bLength = value.toInt();
                else if (key == "InterfaceDescriptor0_bDescriptorType")
                    interfaceDescriptor0_bDescriptorType = value.toInt();
                else if (key == "InterfaceDescriptor0_bInterfaceNumber")
                    interfaceDescriptor0_bInterfaceNumber = value.toInt();
                else if (key == "InterfaceDescriptor0_bAlternateSetting")
                    interfaceDescriptor0_bAlternateSetting = value.toInt();
                else if (key == "InterfaceDescriptor0_bNumEndpoints")
                    interfaceDescriptor0_bNumEndpoints = value.toInt();
                else if (key == "InterfaceDescriptor0_bInterfaceClass")
                    interfaceDescriptor0_bInterfaceClass = value.toInt();
                else if (key == "InterfaceDescriptor0_bInterfaceSubClass")
                    interfaceDescriptor0_bInterfaceSubClass = value.toInt();
                else if (key == "InterfaceDescriptor0_bInterfaceProtocol")
                    interfaceDescriptor0_bInterfaceProtocol = value.toInt();
                else if (key == "InterfaceDescriptor0_iInterface")
                    interfaceDescriptor0_iInterface = value.toInt();

                // Interface Descriptor 1
                else if (key == "InterfaceDescriptor1_bLength")
                    interfaceDescriptor1_bLength = value.toInt();
                else if (key == "InterfaceDescriptor1_bDescriptorType")
                    interfaceDescriptor1_bDescriptorType = value.toInt();
                else if (key == "InterfaceDescriptor1_bInterfaceNumber")
                    interfaceDescriptor1_bInterfaceNumber = value.toInt();
                else if (key == "InterfaceDescriptor1_bAlternateSetting")
                    interfaceDescriptor1_bAlternateSetting = value.toInt();
                else if (key == "InterfaceDescriptor1_bNumEndpoints")
                    interfaceDescriptor1_bNumEndpoints = value.toInt();
                else if (key == "InterfaceDescriptor1_bInterfaceClass")
                    interfaceDescriptor1_bInterfaceClass = value.toInt();
                else if (key == "InterfaceDescriptor1_bInterfaceSubClass")
                    interfaceDescriptor1_bInterfaceSubClass = value.toInt();
                else if (key == "InterfaceDescriptor1_bInterfaceProtocol")
                    interfaceDescriptor1_bInterfaceProtocol = value.toInt();
                else if (key == "InterfaceDescriptor1_iInterface")
                    interfaceDescriptor1_iInterface = value.toInt();

                // HID Descriptor 0
                else if (key == "HIDDescriptor0_bLength")
                    hidDescriptor0_bLength = value.toInt();
                else if (key == "HIDDescriptor0_bDescriptorType")
                    hidDescriptor0_bDescriptorType = value.toInt();
                else if (key == "HIDDescriptor0_bcdHID")
                    hidDescriptor0_bcdHID = strtoul(value.c_str(), NULL, 16);
                else if (key == "HIDDescriptor0_bCountryCode")
                    hidDescriptor0_bCountryCode = value.toInt();
                else if (key == "HIDDescriptor0_bNumDescriptors")
                    hidDescriptor0_bNumDescriptors = value.toInt();
                else if (key == "HIDDescriptor0_bDescriptorType0")
                    hidDescriptor0_bDescriptorType0 = value.toInt();
                else if (key == "HIDDescriptor0_wDescriptorLength")
                    hidDescriptor0_wDescriptorLength = strtoul(value.c_str(), NULL, 16);

                // HID Descriptor 1
                else if (key == "HIDDescriptor1_bLength")
                    hidDescriptor1_bLength = value.toInt();
                else if (key == "HIDDescriptor1_bDescriptorType")
                    hidDescriptor1_bDescriptorType = value.toInt();
                else if (key == "HIDDescriptor1_bcdHID")
                    hidDescriptor1_bcdHID = strtoul(value.c_str(), NULL, 16);
                else if (key == "HIDDescriptor1_bCountryCode")
                    hidDescriptor1_bCountryCode = value.toInt();
                else if (key == "HIDDescriptor1_bNumDescriptors")
                    hidDescriptor1_bNumDescriptors = value.toInt();
                else if (key == "HIDDescriptor1_bDescriptorType0")
                    hidDescriptor1_bDescriptorType0 = value.toInt();
                else if (key == "HIDDescriptor1_wDescriptorLength")
                    hidDescriptor1_wDescriptorLength = strtoul(value.c_str(), NULL, 16);

                // Endpoint Descriptor 0
                else if (key == "EndpointDescriptor0_bLength")
                    endpointDescriptor0_bLength = value.toInt();
                else if (key == "EndpointDescriptor0_bDescriptorType")
                    endpointDescriptor0_bDescriptorType = value.toInt();
                else if (key == "EndpointDescriptor0_bEndpointAddress")
                    endpointDescriptor0_bEndpointAddress = value.toInt();
                else if (key == "EndpointDescriptor0_bmAttributes")
                    endpointDescriptor0_bmAttributes = value.toInt();
                else if (key == "EndpointDescriptor0_wMaxPacketSize")
                    endpointDescriptor0_wMaxPacketSize = strtoul(value.c_str(), NULL, 16);
                else if (key == "EndpointDescriptor0_bInterval")
                    endpointDescriptor0_bInterval = value.toInt();

                // Endpoint Descriptor 1
                else if (key == "EndpointDescriptor1_bLength")
                    endpointDescriptor1_bLength = value.toInt();
                else if (key == "EndpointDescriptor1_bDescriptorType")
                    endpointDescriptor1_bDescriptorType = value.toInt();
                else if (key == "EndpointDescriptor1_bEndpointAddress")
                    endpointDescriptor1_bEndpointAddress = value.toInt();
                else if (key == "EndpointDescriptor1_bmAttributes")
                    endpointDescriptor1_bmAttributes = value.toInt();
                else if (key == "EndpointDescriptor1_wMaxPacketSize")
                    endpointDescriptor1_wMaxPacketSize = strtoul(value.c_str(), NULL, 16);
                else if (key == "EndpointDescriptor1_bInterval")
                    endpointDescriptor1_bInterval = value.toInt();
            }
        }
    }
    configFile.close();
}

void saveToSpiffs()
{
    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial0.println("Failed to mount file system");
        return;
    }

    // Open the config file for writing
    File configFile = SPIFFS.open(configFilePath, FILE_WRITE);
    if (!configFile)
    {
        Serial0.println("Failed to open config file for writing");
        return;
    }

    // Write configuration settings
    configFile.printf(
        "VID=%s\nPID=%s\nUSBVersion=%s\nFirmwareVersion=%s\nProductName=%s\nManufacturerName=%s\nSerialNumber=%s\n"
        "USBPower=%s\nUSBAttributes=%s\nDescriptor4=%s\nLanguageID=0x%04X\n",
        vid.c_str(), pid.c_str(), usbVersion.c_str(), firmwareVersion.c_str(),
        productName.c_str(), manufacturerName.c_str(), serialNumber.c_str(),
        usbPower.c_str(), usbAttributes.c_str(), descriptor4.c_str(), (uint16_t)languageID);

    configFile.printf(
        "DeviceDescriptor_bLength=%d\nDeviceDescriptor_bDescriptorType=%d\nDeviceDescriptor_bcdUSB=0x%04X\n"
        "DeviceDescriptor_bDeviceClass=%d\nDeviceDescriptor_bDeviceSubClass=%d\nDeviceDescriptor_bDeviceProtocol=%d\n"
        "DeviceDescriptor_bMaxPacketSize0=%d\nDeviceDescriptor_idVendor=0x%04X\nDeviceDescriptor_idProduct=0x%04X\n"
        "DeviceDescriptor_bcdDevice=0x%04X\nDeviceDescriptor_iManufacturer=%d\nDeviceDescriptor_iProduct=%d\n"
        "DeviceDescriptor_iSerialNumber=%d\nDeviceDescriptor_bNumConfigurations=%d\n",
        deviceDescriptor_bLength, deviceDescriptor_bDescriptorType, deviceDescriptor_bcdUSB, deviceDescriptor_bDeviceClass,
        deviceDescriptor_bDeviceSubClass, deviceDescriptor_bDeviceProtocol, deviceDescriptor_bMaxPacketSize0,
        deviceDescriptor_idVendor, deviceDescriptor_idProduct, deviceDescriptor_bcdDevice, deviceDescriptor_iManufacturer,
        deviceDescriptor_iProduct, deviceDescriptor_iSerialNumber, deviceDescriptor_bNumConfigurations);

    configFile.printf(
        "ConfigurationDescriptor_bLength=%d\nConfigurationDescriptor_bDescriptorType=%d\nConfigurationDescriptor_wTotalLength=0x%04X\n"
        "ConfigurationDescriptor_bNumInterfaces=%d\nConfigurationDescriptor_bConfigurationValue=%d\n"
        "ConfigurationDescriptor_iConfiguration=%d\nConfigurationDescriptor_bmAttributes=%d\nConfigurationDescriptor_MaxPower=%d\n",
        configurationDescriptor_bLength, configurationDescriptor_bDescriptorType, configurationDescriptor_wTotalLength,
        configurationDescriptor_bNumInterfaces, configurationDescriptor_bConfigurationValue, configurationDescriptor_iConfiguration,
        configurationDescriptor_bmAttributes, configurationDescriptor_MaxPower);

    configFile.printf(
        "InterfaceDescriptor0_bLength=%d\nInterfaceDescriptor0_bDescriptorType=%d\nInterfaceDescriptor0_bInterfaceNumber=%d\n"
        "InterfaceDescriptor0_bAlternateSetting=%d\nInterfaceDescriptor0_bNumEndpoints=%d\nInterfaceDescriptor0_bInterfaceClass=%d\n"
        "InterfaceDescriptor0_bInterfaceSubClass=%d\nInterfaceDescriptor0_bInterfaceProtocol=%d\nInterfaceDescriptor0_iInterface=%d\n",
        interfaceDescriptor0_bLength, interfaceDescriptor0_bDescriptorType, interfaceDescriptor0_bInterfaceNumber,
        interfaceDescriptor0_bAlternateSetting, interfaceDescriptor0_bNumEndpoints, interfaceDescriptor0_bInterfaceClass,
        interfaceDescriptor0_bInterfaceSubClass, interfaceDescriptor0_bInterfaceProtocol, interfaceDescriptor0_iInterface);

    configFile.printf(
        "HIDDescriptor0_bLength=%d\nHIDDescriptor0_bDescriptorType=%d\nHIDDescriptor0_bcdHID=0x%04X\n"
        "HIDDescriptor0_bCountryCode=%d\nHIDDescriptor0_bNumDescriptors=%d\nHIDDescriptor0_bDescriptorType0=%d\n"
        "HIDDescriptor0_wDescriptorLength=0x%04X\n",
        hidDescriptor0_bLength, hidDescriptor0_bDescriptorType, hidDescriptor0_bcdHID, hidDescriptor0_bCountryCode,
        hidDescriptor0_bNumDescriptors, hidDescriptor0_bDescriptorType0, hidDescriptor0_wDescriptorLength);

    configFile.printf(
        "EndpointDescriptor0_bLength=%d\nEndpointDescriptor0_bDescriptorType=%d\nEndpointDescriptor0_bEndpointAddress=%d\n"
        "EndpointDescriptor0_bmAttributes=%d\nEndpointDescriptor0_wMaxPacketSize=0x%04X\nEndpointDescriptor0_bInterval=%d\n",
        endpointDescriptor0_bLength, endpointDescriptor0_bDescriptorType, endpointDescriptor0_bEndpointAddress,
        endpointDescriptor0_bmAttributes, endpointDescriptor0_wMaxPacketSize, endpointDescriptor0_bInterval);

    configFile.printf(
        "InterfaceDescriptor1_bLength=%d\nInterfaceDescriptor1_bDescriptorType=%d\nInterfaceDescriptor1_bInterfaceNumber=%d\n"
        "InterfaceDescriptor1_bAlternateSetting=%d\nInterfaceDescriptor1_bNumEndpoints=%d\nInterfaceDescriptor1_bInterfaceClass=%d\n"
        "InterfaceDescriptor1_bInterfaceSubClass=%d\nInterfaceDescriptor1_bInterfaceProtocol=%d\nInterfaceDescriptor1_iInterface=%d\n",
        interfaceDescriptor1_bLength, interfaceDescriptor1_bDescriptorType, interfaceDescriptor1_bInterfaceNumber,
        interfaceDescriptor1_bAlternateSetting, interfaceDescriptor1_bNumEndpoints, interfaceDescriptor1_bInterfaceClass,
        interfaceDescriptor1_bInterfaceSubClass, interfaceDescriptor1_bInterfaceProtocol, interfaceDescriptor1_iInterface);

    configFile.printf(
        "HIDDescriptor1_bLength=%d\nHIDDescriptor1_bDescriptorType=%d\nHIDDescriptor1_bcdHID=0x%04X\n"
        "HIDDescriptor1_bCountryCode=%d\nHIDDescriptor1_bNumDescriptors=%d\nHIDDescriptor1_bDescriptorType0=%d\n"
        "HIDDescriptor1_wDescriptorLength=0x%04X\n",
        hidDescriptor1_bLength, hidDescriptor1_bDescriptorType, hidDescriptor1_bcdHID, hidDescriptor1_bCountryCode,
        hidDescriptor1_bNumDescriptors, hidDescriptor1_bDescriptorType0, hidDescriptor1_wDescriptorLength);

    configFile.printf(
        "EndpointDescriptor1_bLength=%d\nEndpointDescriptor1_bDescriptorType=%d\nEndpointDescriptor1_bEndpointAddress=%d\n"
        "EndpointDescriptor1_bmAttributes=%d\nEndpointDescriptor1_wMaxPacketSize=0x%04X\nEndpointDescriptor1_bInterval=%d\n",
        endpointDescriptor1_bLength, endpointDescriptor1_bDescriptorType, endpointDescriptor1_bEndpointAddress,
        endpointDescriptor1_bmAttributes, endpointDescriptor1_wMaxPacketSize, endpointDescriptor1_bInterval);

    configFile.close();
}

void printConfigFile()
{
    File configFile = SPIFFS.open(configFilePath, FILE_READ);
    if (!configFile)
    {
        Serial0.println("Failed to open config file");
        return;
    }

    Serial0.println("Contents of the config file:");
    while (configFile.available())
    {
        String line = configFile.readStringUntil('\n');
        Serial0.println(line);
    }
    configFile.close();
}

void deleteConfig()
{
    if (SPIFFS.exists(configFilePath))
    {
        if (SPIFFS.remove(configFilePath))
        {
            Serial0.println("Config file deleted successfully.");
        }
        else
        {
            Serial0.println("Failed to delete config file.");
        }
    }
    else
    {
        Serial0.println("Config file does not exist.");
    }
    ESP.restart();
}
