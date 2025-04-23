#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include "config/pins.h"
#include "assets/qr_codes/wifi_qr_code.h"
#include "ui/menu/menu.h"
#include <TFT_eSPI.h>

bool isTouched();
long readCapacitiveSensor(int pin);
void touch_loop();

#endif // TOUCH_H