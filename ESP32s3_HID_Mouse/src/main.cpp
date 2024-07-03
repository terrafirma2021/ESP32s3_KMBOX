#include "EspUsbHost.h"

EspUsbHost usbHost;


void setup()
{
  Serial0.begin(230400);
  Serial1.begin(4000000, SERIAL_8N1, 2, 1); // Swap RX/TX from ESP A  SERIAL_8N1, 2, 1
  delay(500);
  usbHost.begin();
}

void loop()
{
// Clean 
}
