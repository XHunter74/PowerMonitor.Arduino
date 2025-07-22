#include <Arduino.h>
#include "led_control.h"

int ledState = 0;

void blinkLed()
{
  digitalWrite(13, ledState ? LOW : HIGH);
  ledState = !ledState;
}
