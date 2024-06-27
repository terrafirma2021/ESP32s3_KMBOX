#include "EspUsbHost.h"

class MyEspUsbHost : public EspUsbHost
{
  void onMouseButtons(hid_mouse_report_t report, uint8_t last_buttons)
  {
    // LEFT
    if (!(last_buttons & MOUSE_BUTTON_LEFT) && (report.buttons & MOUSE_BUTTON_LEFT))
    {
      Serial0.print("km.left(1)\n");
    }
    if ((last_buttons & MOUSE_BUTTON_LEFT) && !(report.buttons & MOUSE_BUTTON_LEFT))
    {
      Serial0.print("km.left(0)\n");
    }

    // RIGHT
    if (!(last_buttons & MOUSE_BUTTON_RIGHT) && (report.buttons & MOUSE_BUTTON_RIGHT))
    {
      Serial0.print("km.right(1)\n");
    }
    if ((last_buttons & MOUSE_BUTTON_RIGHT) && !(report.buttons & MOUSE_BUTTON_RIGHT))
    {
      Serial0.print("km.right(0)\n");
    }

    // MIDDLE
    if (!(last_buttons & MOUSE_BUTTON_MIDDLE) && (report.buttons & MOUSE_BUTTON_MIDDLE))
    {
      Serial0.print("km.middle(1)\n");
    }
    if ((last_buttons & MOUSE_BUTTON_MIDDLE) && !(report.buttons & MOUSE_BUTTON_MIDDLE))
    {
      Serial0.print("km.middle(0)\n");
    }

    // FORWARD
    if (!(last_buttons & MOUSE_BUTTON_FORWARD) && (report.buttons & MOUSE_BUTTON_FORWARD))
    {
      Serial0.print("km.side1(1)\n");
    }
    if ((last_buttons & MOUSE_BUTTON_FORWARD) && !(report.buttons & MOUSE_BUTTON_FORWARD))
    {
      Serial0.print("km.side1(0)\n");
    }
  
    // BACKWARD
    if (!(last_buttons & MOUSE_BUTTON_BACKWARD) && (report.buttons & MOUSE_BUTTON_BACKWARD))
    {
      Serial0.print("km.side2(1)\n");
    }
    if ((last_buttons & MOUSE_BUTTON_BACKWARD) && !(report.buttons & MOUSE_BUTTON_BACKWARD))
    {
      Serial0.print("km.side2(0)\n");
    }
  }

  void onMouseMove(hid_mouse_report_t report)
  {
    if (report.wheel != 0)
    {
      Serial0.printf("km.wheel(%d)\n", report.wheel);
    }
    else
    {
      Serial0.printf("km.move(%d,%d)\n", report.x, report.y);
    }
  }
};

MyEspUsbHost usbHost;

void setup()
{
  //Serial0.begin(115200);
  Serial0.begin(115200, SERIAL_8N1, 2, 1); // Swap RX/TX from ESP A  SERIAL_8N1, 2, 1
  delay(500);
  usbHost.begin();
}

void loop()
{
  usbHost.task();
}
