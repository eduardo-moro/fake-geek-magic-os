#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include "config/pins.h"
#include "assets/qr_codes/wifi_qr_code.h"
#include "ui/menu/menu.h"
#include <TFT_eSPI.h>

extern bool wasTouched;
extern bool longPressDisplayed;
extern bool waitingForSecondTap;
extern bool potentialSingleClick;
extern unsigned long lastTapTime;
extern unsigned long touchStartTime;
extern int current_menu;
extern TFT_eSPI tft;
extern String route;
extern unsigned long lastUserActivity;

bool isTouched();
long readCapacitiveSensor(int pin);
void touch_loop();

void detectMenuTouch();
void handleMenuClick();
void handleMenuPress();
void handleMenuDoubleClick();
void handleMenuRelease();
void handleClockQuit();
void doNothing();
void registerUserActivity();

typedef struct MenuCommand
{
    void (*onClick)();
    void (*onLongPress)();
    void (*onDoubleClick)();
    void (*onRelease)();
};

#endif // TOUCH_H