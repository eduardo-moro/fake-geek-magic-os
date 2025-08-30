#ifndef MENU_H
#define MENU_H

#include "arduino.h"
#include "TFT_eSPI.h"
#include "config/positions.hpp"
#include "assets/menu/secondary_menu_bg.hpp"
#include "assets/menu/menu_bg.hpp"
#include "config/menu_items.hpp"
#include "display/display.hpp"

#include "../assets/menu/icons/wifi_icon.hpp"
#include "../assets/menu/icons/bright_icon.hpp"
#include "../assets/menu/icons/clock_icon.hpp"
#include "../assets/menu/icons/settings_icon.hpp"
#include "../assets/qr_codes/wifi_qr_code.hpp"
#include "../ui/animation/animation.hpp"
#include "../web/web.hpp"
#include "main.hpp"

#define CLOCK_MENU_INDEX 0
#define WIFI_MENU_INDEX 1
#define WIFI_SELECT_INDEX 2
#define BRIGHT_MENU_INDEX 3
#define TIMEBOX_MENU_INDEX 4

extern TFT_eSPI tft;
extern bool wasTouched;
extern int current_menu;
extern int valid_menu_items;
extern String route;
extern int initial_timebox;
extern int timebox;

void drawMenuBackground();
void set_open_menu(int menu);
void nextMenu();
void prevMenu();
void handleWifiClick();
void handleClockClick();
void doNothing();
void updateMenuLabels();
void initializeMenu();
void handleTimeboxClick();
void handleBrightClick();
void handleSelectWifi();
void handleWifiSelected();
void returnToMenu();

#endif // MENU_H