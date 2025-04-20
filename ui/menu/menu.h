#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

void drawMenuBackground();
void drawMenuText(const char *text);
void set_open_menu(int menu);

#endif