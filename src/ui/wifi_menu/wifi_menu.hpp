#ifndef WIFI_MENU_H
#define WIFI_MENU_H

#include <Arduino.h>
#include "../list_menu/list_menu.hpp"
#include "../../web/web.hpp"

extern String route;

// Initialize and show the WiFi selection menu
void showWifiSelectionMenu();

// Handler for going back to main menu
void handleWifiMenuBack(const String& value);

// Handler for connecting to selected network
void handleWifiConnect(const String& value);

#endif // WIFI_MENU_H
