#pragma once
#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "SPIFFS.h"
#include <KMBOX.h>
#include <menu.h>
#include <spiffsconf.h>
#include <USBSetup.h>

extern USBHIDMouse Mouse;

// Function prototypes
void handleSerialComs();
void processSerial(HardwareSerial &serial);

// Bool
bool debugEnabled = false;

// Mouse position tracking
int16_t mouseX = 0;
int16_t mouseY = 0;

// Constants for command mappings
const String MOVE_COMMAND = "km.move";
const String MOVETO_COMMAND = "km.moveto";
const String GETPOS_COMMAND = "km.getpos";
const String BUTTON1_DOWN_COMMAND = "km.left(1)";
const String BUTTON1_UP_COMMAND = "km.left(0)";
const String BUTTON2_DOWN_COMMAND = "km.right(1)";
const String BUTTON2_UP_COMMAND = "km.right(0)";
const String BUTTON3_DOWN_COMMAND = "km.middle(1)";
const String BUTTON3_UP_COMMAND = "km.middle(0)";
const String BUTTON4_DOWN_COMMAND = "km.side1(1)";  // Are these the right commands
const String BUTTON4_UP_COMMAND = "km.side1(0)";    // ??
const String BUTTON5_DOWN_COMMAND = "km.side2(1)";  // ??
const String BUTTON5_UP_COMMAND = "km.side2(0)";    // ??
const String WHEEL_COMMAND = "km.wheel";

// Open the menu
const String CHANGE_DEVICE_COMMAND = "menu";

// Debug Enable
const String DEBUG_ON = "debug on";
const String DEBUG_OFF = "debug off";

void debugSerial(const String &message1, const String &message2 = "")
{
    if (debugEnabled)
    {
        Serial0.print(message1);
        Serial0.println(message2);
    }
}

void handleSerialComs()
{
    if (Serial0.available())
    {
        processSerial(Serial0);
    }
    if (Serial2.available())
    {
        processSerial(Serial2);
    }
}

void processSerial(HardwareSerial &serial)
{
    if (serial.available())
    {
        String command = serial.readStringUntil('\n');
        command.trim();
        debugSerial("Received command: '", command + "'");

        if (command.startsWith(MOVE_COMMAND))
        {
            debugSerial("Executing MOVE_COMMAND");

            // Extract coordinates for mouse movement
            command.remove(0, MOVE_COMMAND.length() + 1);
            command.replace("(", "");
            command.replace(")", "");

            // Split the command into parameters
            int params[5] = {0, 0, 0, 0, 0};
            int paramIndex = 0;
            int lastCommaIndex = -1;
            for (int i = 0; i < command.length(); i++)
            {
                if (command[i] == ',')
                {
                    params[paramIndex] = command.substring(lastCommaIndex + 1, i).toInt();
                    lastCommaIndex = i;
                    paramIndex++;
                    if (paramIndex >= 5)
                        break;
                }
            }
            // Add the last parameter if any
            if (lastCommaIndex < command.length() - 1 && paramIndex < 5)
            {
                params[paramIndex] = command.substring(lastCommaIndex + 1).toInt();
            }

            int16_t xDistance = params[0];
            int16_t yDistance = params[1];
            int steps = params[2] > 0 ? params[2] : 1;
            int16_t refX = params[3];
            int16_t refY = params[4];

            // Handle movement logic
            for (int i = 0; i < steps; i++)
            {
                float t = (float)i / steps;
                float u = 1.0 - t;
                int intermediateX, intermediateY;

                if (refX != 0 || refY != 0)
                {
                    // Quadratic Bezier curve calculation
                    intermediateX = u * u * mouseX + 2 * u * t * refX + t * t * (mouseX + xDistance);
                    intermediateY = u * u * mouseY + 2 * u * t * refY + t * t * (mouseY + yDistance);
                }
                else
                {
                    // Linear interpolation
                    intermediateX = mouseX + t * xDistance;
                    intermediateY = mouseY + t * yDistance;
                }

                Mouse.move(intermediateX - mouseX, intermediateY - mouseY);
                mouseX = intermediateX;
                mouseY = intermediateY;
            }

            // Final move to ensure reaching the exact destination
            Mouse.move((mouseX + xDistance) - mouseX, (mouseY + yDistance) - mouseY);
            mouseX += xDistance;
            mouseY += yDistance;

            debugSerial("Moved mouse by x: " + String(xDistance), " y: " + String(yDistance));
            debugSerial(" in steps: " + String(steps));
        }
        else if (command.startsWith(MOVETO_COMMAND))
        {
            debugSerial("Executing MOVETO_COMMAND");
            // Handle absolute mouse movement
            command.remove(0, MOVETO_COMMAND.length() + 1);
            command.replace("(", "");
            command.replace(")", "");
            int commaIndex = command.indexOf(',');
            if (commaIndex != -1)
            {
                int16_t targetX = command.substring(0, commaIndex).toInt();
                int16_t targetY = command.substring(commaIndex + 1).toInt();

                // Debugging: Print target positions
                debugSerial("Parsed targetX: " + String(targetX));
                debugSerial("Parsed targetY: " + String(targetY));

                // Calculate the distances to move based on the current position
                int16_t xDistance = targetX - mouseX;
                int16_t yDistance = targetY - mouseY;

                // Debugging: Print distances
                debugSerial("Calculated xDistance: " + String(xDistance));
                debugSerial("Calculated yDistance: " + String(yDistance));

                // Move the mouse to the specified absolute position
                Mouse.move(xDistance, yDistance);

                // Update the current absolute position of the mouse
                mouseX = targetX;
                mouseY = targetY;

                debugSerial("Moved mouse to absolute position x: " + String(targetX), " y: " + String(targetY));
            }
            else
            {
                debugSerial("Error parsing MOVETO_COMMAND");
            }
        }
        else if (command == GETPOS_COMMAND)
        {
            debugSerial("Executing GETPOS_COMMAND");

            // Send current mouse position over serial
            debugSerial("X:" + String(mouseX), ", Y:" + String(mouseY));
        }
        else if (command == BUTTON1_DOWN_COMMAND)
        {
            debugSerial("Executing BUTTON1_DOWN_COMMAND");
            Mouse.press(MOUSE_BUTTON_LEFT);
        }
        else if (command == BUTTON1_UP_COMMAND)
        {
            debugSerial("Executing BUTTON1_UP_COMMAND");
            Mouse.release(MOUSE_BUTTON_LEFT);
        }
        else if (command == BUTTON2_DOWN_COMMAND)
        {
            debugSerial("Executing BUTTON2_DOWN_COMMAND");
            Mouse.press(MOUSE_BUTTON_RIGHT);
        }
        else if (command == BUTTON2_UP_COMMAND)
        {
            debugSerial("Executing BUTTON2_UP_COMMAND");
            Mouse.release(MOUSE_BUTTON_RIGHT);
        }
        else if (command == BUTTON3_DOWN_COMMAND)
        {
            debugSerial("Executing BUTTON3_DOWN_COMMAND");
            Mouse.press(MOUSE_BUTTON_MIDDLE);
        }
        else if (command == BUTTON3_UP_COMMAND)
        {
            debugSerial("Executing BUTTON3_UP_COMMAND");
            Mouse.release(MOUSE_BUTTON_MIDDLE);
        }
        else if (command == BUTTON4_DOWN_COMMAND)
        {
            debugSerial("Executing BUTTON4_DOWN_COMMAND");
            Mouse.press(MOUSE_BUTTON_FORWARD);
        }
        else if (command == BUTTON4_UP_COMMAND)
        {
            debugSerial("Executing BUTTON4_UP_COMMAND");
            Mouse.release(MOUSE_BUTTON_FORWARD);
        }
        else if (command == BUTTON5_DOWN_COMMAND)
        {
            debugSerial("Executing BUTTON5_DOWN_COMMAND");
            Mouse.press(MOUSE_BUTTON_BACKWARD);
        }
        else if (command == BUTTON5_UP_COMMAND)
        {
            debugSerial("Executing BUTTON5_UP_COMMAND");
            Mouse.release(MOUSE_BUTTON_BACKWARD);
        }
        else if (command.startsWith(WHEEL_COMMAND))
        {
            debugSerial("Executing WHEEL_COMMAND");
            command.remove(0, WHEEL_COMMAND.length() + 1);
            command.replace("(", "");
            command.replace(")", "");
            int wheelMovement = command.toInt();

            // Move the mouse wheel
            Mouse.move(0, 0, wheelMovement);

            debugSerial("Moved mouse wheel by: " + String(wheelMovement));
        }
        else if (command == CHANGE_DEVICE_COMMAND)
        {
            debugSerial("Executing CHANGE_DEVICE_COMMAND");
            deviceMenu();
        }
        else if (command == DEBUG_ON)
        {
            debugEnabled = true;
            debugSerial("Debug Enabled");
        }
        else if (command == DEBUG_OFF)
        {
            debugSerial("Debug Disabled");
            debugEnabled = false;
        }
        else
        {
            debugSerial("Unknown command: " + command);
        }
    }
}