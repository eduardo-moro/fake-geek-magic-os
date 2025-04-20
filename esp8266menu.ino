#include <TFT_eSPI.h>
#include <SPI.h>

#include "assets/menu/icons/wifi_icon.h"
#include "assets/qr_codes/wifi_qr_code.h"
#include "assets/menu/icons/bright_icon.h"
#include "assets/menu/menu_bg.h"
#include "assets/menu/secondary_menu_bg.h"
#include "config/pins.h"
#include "config/positions.h"
#include "main.h"

#include "io/touch_control/touch.h"
#include "ui/menu/menu.h"
#include "ui/animations/animations.h"

TFT_eSPI tft = TFT_eSPI();

// State definitions
int current_menu = 0;

// Control definitions
bool wasTouched = false;
bool longPressDisplayed = false;
bool waitingForSecondTap = false;
bool potentialSingleClick = false;
unsigned long lastTapTime = 0;
unsigned long touchStartTime = 0;

// Animation state variables
unsigned long lastAnimTime = 0;
int currentAnimFrame = 0;
bool animate = true;

void setup()
{
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, 100);

  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  drawMenuBackground();
  tft.setTextSize(1);
  tft.drawString("12:00", 40, 14);
  drawMenuText("WIFI");
}

void loop()
{
  touch_loop();

  if (animate)
  {
    if (current_menu == 0)
    {
      loop_wifi_icon();
    }
    else
    {
      loop_bright_icon();
    }
  }
}
