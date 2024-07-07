#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "SPIFFS.h"
#include <KMBOX.h>
#include <menu.h>
#include <spiffsconf.h>
#include <USBSetup.h>

// Function prototypes
extern void loadConfig();
extern void saveToSpiffs();
extern void setup();
extern void loop();
extern void handleSerialComs();
extern void InitSpiffs();
extern void InitUSB();
extern void deviceMenu();

void setup() {
  Serial0.begin(115200);
  Serial1.begin(4000000, SERIAL_8N1, 1, 2); // Swap RX/TX from ESP A  SERIAL_8N1, 2, 1
  delay(1000);
  InitSpiffs();
  InitUSB();
  setupDMA();
  Mouse.begin();
}

void loop() {
  handleSerialComs();
  parseDMAData(dmaBuffer0, dmaBufferIndex0);
  parseDMAData(dmaBuffer1, dmaBufferIndex1);
}
