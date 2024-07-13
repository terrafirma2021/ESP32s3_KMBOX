// Command Execution for USBHIDMouse with DMA and Serial Configuration

#pragma once

#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "SPIFFS.h"
#include <menu.h>
#include <spiffsconf.h>
#include <USBSetup.h>
#include "driver/uart.h"
#include "esp_intr_alloc.h"

// Mouse and Command Definitions
USBHIDMouse Mouse;

struct Command {
    enum CommandType {
        MOVE,
        MOVETO,
        GETPOS,
        BUTTON1_DOWN,
        BUTTON1_UP,
        BUTTON2_DOWN,
        BUTTON2_UP,
        BUTTON3_DOWN,
        BUTTON3_UP,
        BUTTON4_DOWN,
        BUTTON4_UP,
        BUTTON5_DOWN,
        BUTTON5_UP,
        WHEEL,
        CHANGE_DEVICE,
        UNKNOWN
    } type;
    int params[2]; // For MOVE and MOVETO commands
    int wheelMovement; // For WHEEL command
    const char* originalCommand;
};

// Mouse position tracking
int16_t mouseX = 0;
int16_t mouseY = 0;

// DMA and Serial configuration
#define BUFFER_SIZE 2046
volatile uint8_t dmaBuffer0[BUFFER_SIZE];
volatile uint8_t dmaBuffer1[BUFFER_SIZE];
volatile size_t dmaBufferHead0 = 0;
volatile size_t dmaBufferTail0 = 0;
volatile size_t dmaBufferHead1 = 0;
volatile size_t dmaBufferTail1 = 0;

// Function Declarations
void IRAM_ATTR onDMAReceive0(void *arg);
void IRAM_ATTR onDMAReceive1(void *arg);
void setupDMA();
void processDMAData(volatile uint8_t* buffer, volatile size_t& head, volatile size_t& tail);
void processCommand(const Command& command);
Command parseCommand(const char *command);

// Command handler function declarations
void handleMove(const Command& cmd);
void handleMoveto(const Command& cmd);
void handleMouseButton(uint8_t button, bool press);
void handleMouseWheel(int wheelMovement);
void handleMouseWheelWrapper(const Command& cmd);
void handleGetPos(const Command& cmd);
void handleChangeDevice(const Command& cmd);
void handleUnknown(const Command &cmd);

// Device Menu State
bool deviceMenuActive = false;

// Setup DMA
void setupDMA() {
    Serial1.onReceive([]() {
        onDMAReceive1(nullptr);
    });

    Serial0.onReceive([]() {
        onDMAReceive0(nullptr);
    });
}

// ISR for Serial1
void IRAM_ATTR onDMAReceive1(void *arg) {
    while (Serial1.available()) {
        uint8_t byte = Serial1.read();
        size_t nextHead = (dmaBufferHead1 + 1) % BUFFER_SIZE;
        if (nextHead != dmaBufferTail1) {
            dmaBuffer1[dmaBufferHead1] = byte;
            dmaBufferHead1 = nextHead;
        } else {
            Serial0.println("onDMAReceive1: Buffer overflow");
        }
    }
}

// ISR for Serial0
void IRAM_ATTR onDMAReceive0(void *arg) {
    while (Serial0.available()) {
        uint8_t byte = Serial0.read();
        size_t nextHead = (dmaBufferHead0 + 1) % BUFFER_SIZE;
        if (nextHead != dmaBufferTail0) {
            dmaBuffer0[dmaBufferHead0] = byte;
            dmaBufferHead0 = nextHead;
        } else {
            Serial0.println("onDMAReceive0: Buffer overflow");
        }
    }
}

// Process DMA Data
void processDMAData(volatile uint8_t* buffer, volatile size_t& head, volatile size_t& tail) {
    static char command[100];
    static size_t commandIndex = 0;

    while (tail != head) {
        char byte = buffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;

        if (byte == '\n' || byte == '\r') {
            if (commandIndex > 0) {
                command[commandIndex] = '\0';

                if (strncmp(command, "km.", 3) == 0) {
                    Command cmd = parseCommand(command);
                    processCommand(cmd);
                } else if (deviceMenuActive) {
                    deviceMenu(command);
                } else {
                    Command cmd = parseCommand(command);
                    processCommand(cmd);
                }
                commandIndex = 0;
            }
        } else if (commandIndex < sizeof(command) - 1) {
            command[commandIndex++] = byte;
        }
    }
}

// Parse Command
Command parseCommand(const char *command) {
    Command cmd;
    cmd.type = Command::UNKNOWN;

    if (strncmp(command, "km.move", strlen("km.move")) == 0) {
        cmd.type = Command::MOVE;
        sscanf(command + strlen("km.move") + 1, "%d,%d", &cmd.params[0], &cmd.params[1]);
    } else if (strncmp(command, "km.moveto", strlen("km.moveto")) == 0) {
        cmd.type = Command::MOVETO;
        sscanf(command + strlen("km.moveto") + 1, "%d,%d", &cmd.params[0], &cmd.params[1]);
    } else if (strcmp(command, "km.getpos") == 0) {
        cmd.type = Command::GETPOS;
    } else if (strcmp(command, "km.left(1)") == 0) {
        cmd.type = Command::BUTTON1_DOWN;
    } else if (strcmp(command, "km.left(0)") == 0) {
        cmd.type = Command::BUTTON1_UP;
    } else if (strcmp(command, "km.right(1)") == 0) {
        cmd.type = Command::BUTTON2_DOWN;
    } else if (strcmp(command, "km.right(0)") == 0) {
        cmd.type = Command::BUTTON2_UP;
    } else if (strcmp(command, "km.middle(1)") == 0) {
        cmd.type = Command::BUTTON3_DOWN;
    } else if (strcmp(command, "km.middle(0)") == 0) {
        cmd.type = Command::BUTTON3_UP;
    } else if (strcmp(command, "km.side1(1)") == 0) {
        cmd.type = Command::BUTTON4_DOWN;
    } else if (strcmp(command, "km.side1(0)") == 0) {
        cmd.type = Command::BUTTON4_UP;
    } else if (strcmp(command, "km.side2(1)") == 0) {
        cmd.type = Command::BUTTON5_DOWN;
    } else if (strcmp(command, "km.side2(0)") == 0) {
        cmd.type = Command::BUTTON5_UP;
    } else if (strncmp(command, "km.wheel", strlen("km.wheel")) == 0) {
        cmd.type = Command::WHEEL;
        sscanf(command + strlen("km.wheel") + 1, "%d", &cmd.wheelMovement);
    } else if (strcmp(command, "menu") == 0) {
        cmd.type = Command::CHANGE_DEVICE;
    }
    return cmd;
}

// Command Handlers
typedef void (*CommandHandler)(const Command&);

CommandHandler handlers[] = {
    handleMove,         // MOVE
    handleMoveto,       // MOVETO
    handleGetPos,       // GETPOS
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_LEFT, true); },  // BUTTON1_DOWN
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_LEFT, false); }, // BUTTON1_UP
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_RIGHT, true); }, // BUTTON2_DOWN
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_RIGHT, false); }, // BUTTON2_UP
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_MIDDLE, true); }, // BUTTON3_DOWN
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_MIDDLE, false); }, // BUTTON3_UP
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_FORWARD, true); }, // BUTTON4_DOWN
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_FORWARD, false); }, // BUTTON4_UP
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_BACKWARD, true); }, // BUTTON5_DOWN
    [](const Command& cmd) { handleMouseButton(MOUSE_BUTTON_BACKWARD, false); }, // BUTTON5_UP
    handleMouseWheelWrapper,   // WHEEL
    handleChangeDevice,        // CHANGE_DEVICE
    handleUnknown              // UNKNOWN
};

// Define the command handler functions
void handleMove(const Command& cmd) {
    Mouse.move(cmd.params[0], cmd.params[1]);
    mouseX += cmd.params[0];
    mouseY += cmd.params[1];
}

void handleMoveto(const Command& cmd) {
    Mouse.move(cmd.params[0] - mouseX, cmd.params[1] - mouseY);
    mouseX = cmd.params[0];
    mouseY = cmd.params[1];
}

void handleMouseButton(uint8_t button, bool press) {
    if (press)
        Mouse.press(button);
    else
        Mouse.release(button);
}

void handleMouseWheel(int wheelMovement) {
    Mouse.move(0, 0, wheelMovement);
}

void handleMouseWheelWrapper(const Command& cmd) {
    handleMouseWheel(cmd.wheelMovement);
}

void handleGetPos(const Command& cmd) {
    Serial0.println("km.pos(" + String(mouseX) + "," + String(mouseY) + ")");
}

void handleChangeDevice(const Command& cmd) {
    deviceMenu(cmd.originalCommand); // Passes the original command string to deviceMenu
}

void handleUnknown(const Command &cmd) {
    Serial0.println("ESP uart bug hot fix");

    uart_disable_intr_mask(UART_NUM_1, 0xFFFFFFFF);
    uart_flush_input(UART_NUM_1);
    vTaskDelay(pdMS_TO_TICKS(1)); 
    uart_enable_intr_mask(UART_NUM_1, 0xFFFFFFFF);
}

// Process Command
void processCommand(const Command& cmd) {
    if (cmd.type < sizeof(handlers) / sizeof(handlers[0]) && handlers[cmd.type]) {
        handlers[cmd.type](cmd);
    }
}
