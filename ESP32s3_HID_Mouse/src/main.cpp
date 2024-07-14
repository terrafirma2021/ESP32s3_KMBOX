#include "EspUsbHost.h"

EspUsbHost usbHost;


void setup()
{
  Serial0.begin(4000000);
  Serial1.begin(4000000, SERIAL_8N1, 2, 1); // Swap RX/TX from ESP A  SERIAL_8N1, 2, 1
  usbHost.begin();
  delay(1000);
}

void loop()
{
// Clean 
}
