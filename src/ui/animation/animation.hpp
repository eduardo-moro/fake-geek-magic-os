#ifndef ANIMATION_H
#define ANIMATION_H

#include "arduino.h"
#include "TFT_eSPI.h"
#include "cstdint"
#include "./assets/menu/icons/wifi_icon.hpp"
#include "./assets/menu/icons/bright_icon.hpp"
#include "config/positions.hpp"
#include "config/menu_items.hpp"
#include "./assets/ui/battery.hpp"
#include "ui/menu/menu.hpp"

extern TFT_eSPI tft;
extern unsigned long lastAnimTime;
extern unsigned long lastAnimTimeBat;
extern int currentAnimFrame;
extern int current_menu;

void loop_wifi_icon();
void loop_icon();
void loop_battery();

#endif // ANIMATION_H