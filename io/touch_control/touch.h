#pragma once
#include <Arduino.h>

bool isTouched();
long readCapacitiveSensor(int pin);
void touch_loop();