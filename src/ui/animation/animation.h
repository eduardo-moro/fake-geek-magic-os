#ifndef ANIMATION_H
#define ANIMATION_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdint>
#include "assets/menu/icons/wifi_icon.h"
#include "assets/menu/icons/bright_icon.h"
#include "config/positions.h"
#include "config/menu_items.h"
#include "assets/ui/battery.h"
#include <TJpg_Decoder.h>


void loop_wifi_icon();
void loop_bright_icon();
void loop_icon();
void loop_battery();
void animate_splash_screen();
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);

#endif // ANIMATION_H