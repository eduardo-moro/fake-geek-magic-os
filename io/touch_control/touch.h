#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>

bool isTouched();
long readCapacitiveSensor(int pin);
void touch_loop();

#endif