#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config/positions.h"
#include "assets/menu/secondary_menu_bg.h"
#include "assets/menu/menu_bg.h"
#include "config/menu_items.h"
#include <display/display.h>

#include <menu/icons/wifi_icon.h>
#include <menu/icons/bright_icon.h>
#include <menu/icons/clock_icon.h>
#include <menu/icons/settings_icon.h>
#include <qr_codes/wifi_qr_code.h>
#include "ui/animation/animation.h"
#include <main.h>
#include <web.h>

#define WIFI_MENU_INDEX 0
#define BRIGHT_MENU_INDEX 1
#define SETTINGS_MENU_INDEX 2
#define CLOCK_MENU_INDEX 3

extern TFT_eSPI tft;
extern bool wasTouched;
extern int current_menu;
extern String route;

void drawMenuBackground();
void set_open_menu(int menu);
void nextMenu();
void prevMenu();
void handleWifiClick();
void handleClockClick();
void doNothing();
void updateMenuLabels();
void initializeMenu();

#endif // MENU_H