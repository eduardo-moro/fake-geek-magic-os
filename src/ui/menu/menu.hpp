#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "TFT_eSPI.h"
#include "config/positions.hpp"
#include "assets/menu/secondary_menu_bg.hpp"
#include "assets/menu/menu_bg.hpp"
#include "config/menu_items.hpp"
#include "display/display.hpp"
#include "pomodoro/pomodoro.hpp"

#include "../assets/menu/icons/wifi_icon.hpp"
#include "../assets/menu/icons/pomodoro_icon.hpp"
#include "../assets/menu/icons/bright_icon.hpp"
#include "../assets/menu/icons/clock_icon.hpp"
#include "../assets/menu/icons/settings_icon.hpp"
#include "../assets/qr_codes/wifi_qr_code.hpp"
#include "../ui/animation/animation.hpp"
#include "../ui/wifi_menu/wifi_menu.hpp"
#include "../ui/test_menu/test_menu.hpp"
#include "../web/web.hpp"
#include "main.hpp"

#ifdef ESP32
#include "buddy/buddy_app.hpp"
#endif

#define CLOCK_MENU_INDEX     0
#define WIFI_MENU_INDEX      1
#define WIFI_SELECT_INDEX    2
#define BRIGHT_MENU_INDEX    3
#define ROTATION_MENU_INDEX  4
#define TIMEBOX_MENU_INDEX   5
#define POMODORO_MENU_INDEX  6
#define ART_MENU_INDEX       7
#define ANIMATE_MENU_INDEX   8
#define TEST_MENU_INDEX      9
#ifdef ESP32
#define BUDDY_MENU_INDEX     10
#endif

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
void handlePomodoroClick();
void updateMenuLabels();
void initializeMenu();
void handleTimeboxClick();
void handleBrightClick();
void handleRotateClick();
void handleSelectWifiClick();
void handleWifiSelected();
void returnToMenu();
void handleArtClick();
void handleAnimateClick();
void handleTestMenuClick();
#ifdef ESP32
void handleBuddyClick();
#endif

#endif // MENU_H