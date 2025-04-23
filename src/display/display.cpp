#include "display.h"

void setBrightnessPercent(int percent) {
  percent = constrain(percent, 0, 100);
  int brightness = map(percent, 0, 100, 255, 0);
  analogWrite(BACKLIGHT_PIN, brightness);
}