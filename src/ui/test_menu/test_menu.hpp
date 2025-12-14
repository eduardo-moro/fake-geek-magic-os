#ifndef TEST_MENU_H
#define TEST_MENU_H

#include "arduino.h"
#include "../list_menu/list_menu.hpp"

extern String route;

// Show the test menu with 30 items
void showTestMenu();

// Handler for test menu back button
void handleTestMenuBack(const String& value);

// Handler for test menu item selection
void handleTestItemSelect(const String& value);

#endif // TEST_MENU_H
