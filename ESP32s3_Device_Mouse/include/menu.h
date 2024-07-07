#pragma once
#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "SPIFFS.h"
#include <KMBOX.h>
#include <spiffsconf.h>
#include <USBSetup.h>



// Function prototypes
void deviceMenu();
extern void deleteConfig();


// Function to print and change device settings
void deviceMenu() {
while (true) {
        Serial0.println("\nWelcome to the ESP Box V1\n\n");
        Serial0.println("Current Device Settings:");
        Serial0.printf("1. VID: %s\n", vid.c_str());
        Serial0.printf("2. PID: %s\n", pid.c_str());
        Serial0.printf("3. USB Version: %s\n", usbVersion.c_str());
        Serial0.printf("4. Firmware Version: %s\n", firmwareVersion.c_str());
        Serial0.printf("5. Product Name: %s\n", productName.c_str());
        Serial0.printf("6. Manufacturer Name: %s\n", manufacturerName.c_str());
        Serial0.printf("7. Serial Number: %s\n", serialNumber.c_str());
        Serial0.printf("8. USB Power: %s\n", usbPower.c_str());
        Serial0.printf("9. USB Attributes: %s\n", usbAttributes.c_str());
        Serial0.printf("10. Descriptor4: %s\n", descriptor4.c_str());
        Serial0.printf("14. Device Descriptor bLength: %d\n", deviceDescriptor_bLength);
        Serial0.printf("15. Device Descriptor bDescriptorType: %d\n", deviceDescriptor_bDescriptorType);
        Serial0.printf("16. Device Descriptor bcdUSB: 0x%04X\n", deviceDescriptor_bcdUSB);
        Serial0.printf("17. Device Descriptor bDeviceClass: %d\n", deviceDescriptor_bDeviceClass);
        Serial0.printf("18. Device Descriptor bDeviceSubClass: %d\n", deviceDescriptor_bDeviceSubClass);
        Serial0.printf("19. Device Descriptor bDeviceProtocol: %d\n", deviceDescriptor_bDeviceProtocol);
        Serial0.printf("20. Device Descriptor bMaxPacketSize0: %d\n", deviceDescriptor_bMaxPacketSize0);
        Serial0.printf("21. Device Descriptor idVendor: 0x%04X\n", deviceDescriptor_idVendor);
        Serial0.printf("22. Device Descriptor idProduct: 0x%04X\n", deviceDescriptor_idProduct);
        Serial0.printf("23. Device Descriptor bcdDevice: 0x%04X\n", deviceDescriptor_bcdDevice);
        Serial0.printf("24. Device Descriptor iManufacturer: %d\n", deviceDescriptor_iManufacturer);
        Serial0.printf("25. Device Descriptor iProduct: %d\n", deviceDescriptor_iProduct);
        Serial0.printf("26. Device Descriptor iSerialNumber: %d\n", deviceDescriptor_iSerialNumber);
        Serial0.printf("27. Device Descriptor bNumConfigurations: %d\n", deviceDescriptor_bNumConfigurations);
        
        Serial0.printf("28. Configuration Descriptor bLength: %d\n", configurationDescriptor_bLength);
        Serial0.printf("29. Configuration Descriptor bDescriptorType: %d\n", configurationDescriptor_bDescriptorType);
        Serial0.printf("30. Configuration Descriptor wTotalLength: 0x%04X\n", configurationDescriptor_wTotalLength);
        Serial0.printf("31. Configuration Descriptor bNumInterfaces: %d\n", configurationDescriptor_bNumInterfaces);
        Serial0.printf("32. Configuration Descriptor bConfigurationValue: %d\n", configurationDescriptor_bConfigurationValue);
        Serial0.printf("33. Configuration Descriptor iConfiguration: %d\n", configurationDescriptor_iConfiguration);
        Serial0.printf("34. Configuration Descriptor bmAttributes: %d\n", configurationDescriptor_bmAttributes);
        Serial0.printf("35. Configuration Descriptor MaxPower: %d\n", configurationDescriptor_MaxPower);
        
        Serial0.printf("36. Interface Descriptor 0 bLength: %d\n", interfaceDescriptor0_bLength);
        Serial0.printf("37. Interface Descriptor 0 bDescriptorType: %d\n", interfaceDescriptor0_bDescriptorType);
        Serial0.printf("38. Interface Descriptor 0 bInterfaceNumber: %d\n", interfaceDescriptor0_bInterfaceNumber);
        Serial0.printf("39. Interface Descriptor 0 bAlternateSetting: %d\n", interfaceDescriptor0_bAlternateSetting);
        Serial0.printf("40. Interface Descriptor 0 bNumEndpoints: %d\n", interfaceDescriptor0_bNumEndpoints);
        Serial0.printf("41. Interface Descriptor 0 bInterfaceClass: %d\n", interfaceDescriptor0_bInterfaceClass);
        Serial0.printf("42. Interface Descriptor 0 bInterfaceSubClass: %d\n", interfaceDescriptor0_bInterfaceSubClass);
        Serial0.printf("43. Interface Descriptor 0 bInterfaceProtocol: %d\n", interfaceDescriptor0_bInterfaceProtocol);
        Serial0.printf("44. Interface Descriptor 0 iInterface: %d\n", interfaceDescriptor0_iInterface);
        
        Serial0.printf("45. HID Descriptor 0 bLength: %d\n", hidDescriptor0_bLength);
        Serial0.printf("46. HID Descriptor 0 bDescriptorType: %d\n", hidDescriptor0_bDescriptorType);
        Serial0.printf("47. HID Descriptor 0 bcdHID: 0x%04X\n", hidDescriptor0_bcdHID);
        Serial0.printf("48. HID Descriptor 0 bCountryCode: %d\n", hidDescriptor0_bCountryCode);
        Serial0.printf("49. HID Descriptor 0 bNumDescriptors: %d\n", hidDescriptor0_bNumDescriptors);
        Serial0.printf("50. HID Descriptor 0 bDescriptorType0: %d\n", hidDescriptor0_bDescriptorType0);
        Serial0.printf("51. HID Descriptor 0 wDescriptorLength: 0x%04X\n", hidDescriptor0_wDescriptorLength);
        
        Serial0.printf("52. Endpoint Descriptor 0 bLength: %d\n", endpointDescriptor0_bLength);
        Serial0.printf("53. Endpoint Descriptor 0 bDescriptorType: %d\n", endpointDescriptor0_bDescriptorType);
        Serial0.printf("54. Endpoint Descriptor 0 bEndpointAddress: %d\n", endpointDescriptor0_bEndpointAddress);
        Serial0.printf("55. Endpoint Descriptor 0 bmAttributes: %d\n", endpointDescriptor0_bmAttributes);
        Serial0.printf("56. Endpoint Descriptor 0 wMaxPacketSize: 0x%04X\n", endpointDescriptor0_wMaxPacketSize);
        Serial0.printf("57. Endpoint Descriptor 0 bInterval: %d\n", endpointDescriptor0_bInterval);
        
        Serial0.printf("58. Interface Descriptor 1 bLength: %d\n", interfaceDescriptor1_bLength);
        Serial0.printf("59. Interface Descriptor 1 bDescriptorType: %d\n", interfaceDescriptor1_bDescriptorType);
        Serial0.printf("60. Interface Descriptor 1 bInterfaceNumber: %d\n", interfaceDescriptor1_bInterfaceNumber);
        Serial0.printf("61. Interface Descriptor 1 bAlternateSetting: %d\n", interfaceDescriptor1_bAlternateSetting);
        Serial0.printf("62. Interface Descriptor 1 bNumEndpoints: %d\n", interfaceDescriptor1_bNumEndpoints);
        Serial0.printf("63. Interface Descriptor 1 bInterfaceClass: %d\n", interfaceDescriptor1_bInterfaceClass);
        Serial0.printf("64. Interface Descriptor 1 bInterfaceSubClass: %d\n", interfaceDescriptor1_bInterfaceSubClass);
        Serial0.printf("65. Interface Descriptor 1 bInterfaceProtocol: %d\n", interfaceDescriptor1_bInterfaceProtocol);
        Serial0.printf("66. Interface Descriptor 1 iInterface: %d\n", interfaceDescriptor1_iInterface);
        
        Serial0.printf("67. HID Descriptor 1 bLength: %d\n", hidDescriptor1_bLength);
        Serial0.printf("68. HID Descriptor 1 bDescriptorType: %d\n", hidDescriptor1_bDescriptorType);
        Serial0.printf("69. HID Descriptor 1 bcdHID: 0x%04X\n", hidDescriptor1_bcdHID);
        Serial0.printf("70. HID Descriptor 1 bCountryCode: %d\n", hidDescriptor1_bCountryCode);
        Serial0.printf("71. HID Descriptor 1 bNumDescriptors: %d\n", hidDescriptor1_bNumDescriptors);
        Serial0.printf("72. HID Descriptor 1 bDescriptorType0: %d\n", hidDescriptor1_bDescriptorType0);
        Serial0.printf("73. HID Descriptor 1 wDescriptorLength: 0x%04X\n", hidDescriptor1_wDescriptorLength);
        
        Serial0.printf("74. Endpoint Descriptor 1 bLength: %d\n", endpointDescriptor1_bLength);
        Serial0.printf("75. Endpoint Descriptor 1 bDescriptorType: %d\n", endpointDescriptor1_bDescriptorType);
        Serial0.printf("76. Endpoint Descriptor 1 bEndpointAddress: %d\n", endpointDescriptor1_bEndpointAddress);
        Serial0.printf("77. Endpoint Descriptor 1 bmAttributes: %d\n", endpointDescriptor1_bmAttributes);
        Serial0.printf("78. Endpoint Descriptor 1 wMaxPacketSize: 0x%04X\n", endpointDescriptor1_wMaxPacketSize);
        Serial0.printf("79. Endpoint Descriptor 1 bInterval: %d\n", endpointDescriptor1_bInterval);

        Serial0.println("80. Print Current Values");
        Serial0.println("81. Delete Values Stored");
        Serial0.println("82. Exit and Save");
        Serial0.println("Type 'menu' to return to this menu at any time.");
        Serial0.println("Enter the number of the setting you want to change or type 'menu':\n\n");
        while (!Serial0.available());
        String input = Serial0.readString();
        input.trim();

        if (input.equalsIgnoreCase("menu")) {
            continue;
        }

        if (input.length() == 0 || !input.toInt()) {
            Serial0.println("Invalid input, please enter a number or 'menu'.\n");
            continue;
        }

        int option = input.toInt();

        if (option == 80) {
            printConfigFile();
            continue;
        }

        if (option == 81) {
            Serial0.println("This will load default firmware values,\nDo not do this unless you are ready to replace your values.\n");
            Serial0.println("Type 'yes' to delete or anything else to cancel:\n");
            while (!Serial0.available());
            String confirmation = Serial0.readString();
            confirmation.trim();
            if (confirmation.equalsIgnoreCase("yes")) {
                deleteConfig();
                return;
            } else {
                Serial0.println("Operation canceled. Returning to menu...");
                continue;
            }
        }

        if (option == 82) {
            saveToSpiffs();
            ESP.restart();
            return;
        }

        if (option < 1 || option > 79) { // too lazy to adjust the catch all 
            Serial0.println("Invalid input, please enter a number or 'menu'.\n");
            continue;
        }

        Serial0.println("Enter the new value:");
        while (!Serial0.available());
        String newValue = Serial0.readString();
        newValue.trim();

        switch (option) {
            case 1: vid = newValue; break;
            case 2: pid = newValue; break;
            case 3: usbVersion = newValue; break;
            case 4: firmwareVersion = newValue; break;
            case 5: productName = newValue; break;
            case 6: manufacturerName = newValue; break;
            case 7: serialNumber = newValue; break;
            case 8: usbPower = newValue; break;
            case 9: usbAttributes = newValue; break;
            case 10: descriptor4 = newValue; break;
            case 14: deviceDescriptor_bLength = newValue.toInt(); break;
            case 15: deviceDescriptor_bDescriptorType = newValue.toInt(); break;
            case 16: deviceDescriptor_bcdUSB = strtoul(newValue.c_str(), NULL, 16); break;
            case 17: deviceDescriptor_bDeviceClass = newValue.toInt(); break;
            case 18: deviceDescriptor_bDeviceSubClass = newValue.toInt(); break;
            case 19: deviceDescriptor_bDeviceProtocol = newValue.toInt(); break;
            case 20: deviceDescriptor_bMaxPacketSize0 = newValue.toInt(); break;
            case 21: deviceDescriptor_idVendor = strtoul(newValue.c_str(), NULL, 16); break;
            case 22: deviceDescriptor_idProduct = strtoul(newValue.c_str(), NULL, 16); break;
            case 23: deviceDescriptor_bcdDevice = strtoul(newValue.c_str(), NULL, 16); break;
            case 24: deviceDescriptor_iManufacturer = newValue.toInt(); break;
            case 25: deviceDescriptor_iProduct = newValue.toInt(); break;
            case 26: deviceDescriptor_iSerialNumber = newValue.toInt(); break;
            case 27: deviceDescriptor_bNumConfigurations = newValue.toInt(); break;
            case 28: configurationDescriptor_bLength = newValue.toInt(); break;
            case 29: configurationDescriptor_bDescriptorType = newValue.toInt(); break;
            case 30: configurationDescriptor_wTotalLength = strtoul(newValue.c_str(), NULL, 16); break;
            case 31: configurationDescriptor_bNumInterfaces = newValue.toInt(); break;
            case 32: configurationDescriptor_bConfigurationValue = newValue.toInt(); break;
            case 33: configurationDescriptor_iConfiguration = newValue.toInt(); break;
            case 34: configurationDescriptor_bmAttributes = newValue.toInt(); break;
            case 35: configurationDescriptor_MaxPower = newValue.toInt(); break;
            case 36: interfaceDescriptor0_bLength = newValue.toInt(); break;
            case 37: interfaceDescriptor0_bDescriptorType = newValue.toInt(); break;
            case 38: interfaceDescriptor0_bInterfaceNumber = newValue.toInt(); break;
            case 39: interfaceDescriptor0_bAlternateSetting = newValue.toInt(); break;
            case 40: interfaceDescriptor0_bNumEndpoints = newValue.toInt(); break;
            case 41: interfaceDescriptor0_bInterfaceClass = newValue.toInt(); break;
            case 42: interfaceDescriptor0_bInterfaceSubClass = newValue.toInt(); break;
            case 43: interfaceDescriptor0_bInterfaceProtocol = newValue.toInt(); break;
            case 44: interfaceDescriptor0_iInterface = newValue.toInt(); break;
            case 45: hidDescriptor0_bLength = newValue.toInt(); break;
            case 46: hidDescriptor0_bDescriptorType = newValue.toInt(); break;
            case 47: hidDescriptor0_bcdHID = strtoul(newValue.c_str(), NULL, 16); break;
            case 48: hidDescriptor0_bCountryCode = newValue.toInt(); break;
            case 49: hidDescriptor0_bNumDescriptors = newValue.toInt(); break;
            case 50: hidDescriptor0_bDescriptorType0 = newValue.toInt(); break;
            case 51: hidDescriptor0_wDescriptorLength = strtoul(newValue.c_str(), NULL, 16); break;
            case 52: endpointDescriptor0_bLength = newValue.toInt(); break;
            case 53: endpointDescriptor0_bDescriptorType = newValue.toInt(); break;
            case 54: endpointDescriptor0_bEndpointAddress = newValue.toInt(); break;
            case 55: endpointDescriptor0_bmAttributes = newValue.toInt(); break;
            case 56: endpointDescriptor0_wMaxPacketSize = strtoul(newValue.c_str(), NULL, 16); break;
            case 57: endpointDescriptor0_bInterval = newValue.toInt(); break;
            case 58: interfaceDescriptor1_bLength = newValue.toInt(); break;
            case 59: interfaceDescriptor1_bDescriptorType = newValue.toInt(); break;
            case 60: interfaceDescriptor1_bInterfaceNumber = newValue.toInt(); break;
            case 61: interfaceDescriptor1_bAlternateSetting = newValue.toInt(); break;
            case 62: interfaceDescriptor1_bNumEndpoints = newValue.toInt(); break;
            case 63: interfaceDescriptor1_bInterfaceClass = newValue.toInt(); break;
            case 64: interfaceDescriptor1_bInterfaceSubClass = newValue.toInt(); break;
            case 65: interfaceDescriptor1_bInterfaceProtocol = newValue.toInt(); break;
            case 66: interfaceDescriptor1_iInterface = newValue.toInt(); break;
            case 67: hidDescriptor1_bLength = newValue.toInt(); break;
            case 68: hidDescriptor1_bDescriptorType = newValue.toInt(); break;
            case 69: hidDescriptor1_bcdHID = strtoul(newValue.c_str(), NULL, 16); break;
            case 70: hidDescriptor1_bCountryCode = newValue.toInt(); break;
            case 71: hidDescriptor1_bNumDescriptors = newValue.toInt(); break;
            case 72: hidDescriptor1_bDescriptorType0 = newValue.toInt(); break;
            case 73: hidDescriptor1_wDescriptorLength = strtoul(newValue.c_str(), NULL, 16); break;
            case 74: endpointDescriptor1_bLength = newValue.toInt(); break;
            case 75: endpointDescriptor1_bDescriptorType = newValue.toInt(); break;
            case 76: endpointDescriptor1_bEndpointAddress = newValue.toInt(); break;
            case 77: endpointDescriptor1_bmAttributes = newValue.toInt(); break;
            case 78: endpointDescriptor1_wMaxPacketSize = strtoul(newValue.c_str(), NULL, 16); break;
            case 79: endpointDescriptor1_bInterval = newValue.toInt(); break;
        }

        saveToSpiffs();
        Serial0.println("Value accepted and saved.\n");
    }
}

