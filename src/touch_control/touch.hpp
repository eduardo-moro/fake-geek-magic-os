#ifndef TOUCH_H
#define TOUCH_H

#include "arduino.h"
#include "config/pins.hpp"
#include "assets/qr_codes/wifi_qr_code.hpp"
#include "ui/menu/menu.hpp"
#include "TFT_eSPI.h"
#include "map"

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
extern bool is_displaying_image;

bool isTouched();
long readCapacitiveSensor(int pin);
void touch_loop();

void detectMenuTouch();
void handleMenuClick();
void handleMenuPress();
void handleMenuDoubleClick();
void handleMenuRelease();
void handleClockQuit();
void handlePixelQuit();
void handleWifiQrQuit();
void doNothing();
void registerUserActivity();
void updateRoute(const String& route);

typedef struct MenuCommand
{
    void (*onClick)();
    void (*onLongPress)();
    void (*onDoubleClick)();
    void (*onRelease)();
};

#endif // TOUCH_H