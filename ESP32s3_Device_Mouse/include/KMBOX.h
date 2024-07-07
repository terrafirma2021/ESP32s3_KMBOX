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
};

// Mouse position tracking
int16_t mouseX = 0;
int16_t mouseY = 0;

// DMA and Serial configuration
#define UART_NUM_ESP0 UART_NUM_0
#define UART_NUM_ESP1 UART_NUM_1
#define BUFFER_SIZE 128
uint8_t dmaBuffer0[BUFFER_SIZE];
uint8_t dmaBuffer1[BUFFER_SIZE];
volatile size_t dmaBufferIndex0 = 0;
volatile size_t dmaBufferIndex1 = 0;
QueueHandle_t uart0_queue;
QueueHandle_t uart1_queue;

// Timer variables
volatile unsigned long startTime = 0;
volatile unsigned long endTime = 0;

void IRAM_ATTR onDMAReceive0(void *arg);
void IRAM_ATTR onDMAReceive1(void *arg);
void setupDMA();
void handleSerialComs();
void processSerial(HardwareSerial &serial, uint8_t* buffer, volatile size_t &bufferIndex);
void processCommand(const Command& command);
void parseDMAData(uint8_t* buffer, volatile size_t &bufferIndex);
extern void deviceMenu();
Command parseCommand(const char *command);

// Command handlers
void handleMove(const Command& cmd) {
    static unsigned long timeBuffer[20]; // Buffer to store timing values
    static int bufferIndex = 0; // Index for the buffer

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
    deviceMenu();
}

typedef void (*CommandHandler)(const Command&);

CommandHandler handlers[] = {
    handleMove,         // MOVE
    handleMoveto,       // MOVETO
    handleGetPos,       // GETPOS
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_LEFT, true); },  // BUTTON1_DOWN
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_LEFT, false); }, // BUTTON1_UP
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_RIGHT, true); },  // BUTTON2_DOWN
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_RIGHT, false); }, // BUTTON2_UP
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_MIDDLE, true); },  // BUTTON3_DOWN
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_MIDDLE, false); }, // BUTTON3_UP
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_FORWARD, true); },  // BUTTON4_DOWN
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_FORWARD, false); }, // BUTTON4_UP
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_BACKWARD, true); },  // BUTTON5_DOWN
    [](const Command& cmd){ handleMouseButton(MOUSE_BUTTON_BACKWARD, false); }, // BUTTON5_UP
    handleMouseWheelWrapper,   // WHEEL
    handleChangeDevice  // CHANGE_DEVICE
};

void processCommand(const Command& cmd) {
    if (cmd.type < sizeof(handlers) / sizeof(handlers[0]) && handlers[cmd.type]) {
        handlers[cmd.type](cmd);
    } else {
        Serial0.println("Error: MCU's not in sync, Waiting..."); // Caused by booting the other mcu first, 
        //having a startup delay of 1000 on this mcu, 
        //idle serial1 comms will solve in 1000ms (no mouse movement)!!
    }
}

// State machine for command parsing
enum ParseState {
    WAITING_FOR_COMMAND,
    READING_COMMAND,
    EXECUTING_COMMAND
};

ParseState state0 = WAITING_FOR_COMMAND;
ParseState state1 = WAITING_FOR_COMMAND;
Command cmd0;
Command cmd1;
size_t commandIndex0 = 0;
size_t commandIndex1 = 0;

void parseDMAData(uint8_t* buffer, volatile size_t &bufferIndex) {
    while (bufferIndex > 0) {
        uint8_t byte = buffer[--bufferIndex];
        ParseState& state = (buffer == dmaBuffer0) ? state0 : state1;
        Command& cmd = (buffer == dmaBuffer0) ? cmd0 : cmd1;
        size_t& commandIndex = (buffer == dmaBuffer0) ? commandIndex0 : commandIndex1;

        switch (state) {
            case WAITING_FOR_COMMAND:
                cmd.type = static_cast<Command::CommandType>(byte);
                commandIndex = 0;
                state = READING_COMMAND;
                break;

            case READING_COMMAND:
                ((uint8_t*)&cmd)[++commandIndex] = byte;
                if (commandIndex == sizeof(Command) - 1) {
                    state = EXECUTING_COMMAND;
                }
                break;

            case EXECUTING_COMMAND:
                processCommand(cmd);
                state = WAITING_FOR_COMMAND;
                break;
        }
    }
}

void setupDMA() {
    const uart_config_t uart_config0 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    const uart_config_t uart_config1 = {
        .baud_rate = 4000000,  // The baud rate for Serial1
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Setup DMA for UART0
    uart_param_config(UART_NUM_ESP0, &uart_config0);
    uart_driver_install(UART_NUM_ESP0, BUFFER_SIZE * 2, 0, 20, &uart0_queue, 0);
    uart_isr_handle_t handle0;
    uart_isr_register(UART_NUM_ESP0, onDMAReceive0, NULL, ESP_INTR_FLAG_IRAM, &handle0);

    // Setup DMA for UART1
    uart_param_config(UART_NUM_ESP1, &uart_config1);
    uart_driver_install(UART_NUM_ESP1, BUFFER_SIZE * 2, 0, 20, &uart1_queue, 0);
    uart_isr_handle_t handle1;
    uart_isr_register(UART_NUM_ESP1, onDMAReceive1, NULL, ESP_INTR_FLAG_IRAM, &handle1);
}

void IRAM_ATTR onDMAReceive0(void *arg) {
    uart_event_t event;
    while (xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
        if (event.type == UART_DATA) {
            uint8_t byte;
            uart_read_bytes(UART_NUM_ESP0, &byte, 1, portMAX_DELAY);
            dmaBuffer0[dmaBufferIndex0] = byte;
            dmaBufferIndex0 = (dmaBufferIndex0 + 1) % BUFFER_SIZE;
        }
    }
}

void IRAM_ATTR onDMAReceive1(void *arg) {
    uart_event_t event;
    while (xQueueReceive(uart1_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
        if (event.type == UART_DATA) {
            uint8_t byte;
            uart_read_bytes(UART_NUM_ESP1, &byte, 1, portMAX_DELAY);
            dmaBuffer1[dmaBufferIndex1] = byte;
            dmaBufferIndex1 = (dmaBufferIndex1 + 1) % BUFFER_SIZE;
        }
    }
}

void handleSerialComs() {
    if (Serial0.available()) {
        processSerial(Serial0, dmaBuffer0, dmaBufferIndex0);
    }
    if (Serial1.available()) {
        processSerial(Serial1, dmaBuffer1, dmaBufferIndex1);
    }
}

void processSerial(HardwareSerial &serial, uint8_t* buffer, volatile size_t &bufferIndex) {
    static char command[100];
    static size_t commandIndex = 0;

    while (serial.available()) {
        char c = serial.read();
        if (c == '\n' || c == '\r') {
            if (commandIndex > 0) {
                command[commandIndex] = '\0';  // Null-terminate the string
                Command cmd = parseCommand(command);
                processCommand(cmd);  // Process command immediately
                commandIndex = 0;  // Reset command index
            }
        } else if (commandIndex < sizeof(command) - 1) {
            command[commandIndex++] = c;
        }
    }
}

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
