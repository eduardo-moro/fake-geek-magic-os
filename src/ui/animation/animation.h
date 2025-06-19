#ifndef ANIMATION_H
#define ANIMATION_H

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "cstdint"
#include "./assets/menu/icons/wifi_icon.h"
#include "./assets/menu/icons/bright_icon.h"
#include "config/positions.h"
#include "config/menu_items.h"
#include "./assets/ui/battery.h"
#include "ui/menu/menu.h"

extern TFT_eSPI tft;
extern unsigned long lastAnimTime;
extern unsigned long lastAnimTimeBat;
extern int currentAnimFrame;
extern int current_menu;

void loop_wifi_icon();
void loop_icon();
void loop_battery();

#endif // ANIMATION_H