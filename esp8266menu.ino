#include <TFT_eSPI.h>
#include <SPI.h>

#include "wifi_icon.h"
#include "bright_icon.h"
#include "menu_bg.h"
#include "secondary_menu_bg.h"
#include "pins.h"
#include "functions.h"
#include "positions.h"

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

void setup() {
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

void loop() {
  touch_loop();
  
  if ( animate ) {
    if (current_menu == 0) {
      loop_wifi_icon();
    } else {
      loop_bright_icon();
    }
  }
}

void loop_wifi_icon() {
  static const uint8_t* frames[] = {wifi_0_bits, wifi_1_bits, wifi_2_bits, wifi_3_bits};
  animateIcon(frames, 4);
}

void loop_bright_icon() {
  static const uint8_t* frames[] = {bright_0_bits, bright_1_bits, bright_2_bits, bright_3_bits};
  animateIcon(frames, 4);
}

void animateIcon(const uint8_t* frames[], int frameCount) {
  if (millis() - lastAnimTime >= icon_pos.anim_delay) {
    lastAnimTime = millis();
    tft.drawXBitmap(icon_pos.x, icon_pos.y, frames[currentAnimFrame], icon_pos.w, icon_pos.h, TFT_BLACK, TFT_WHITE);
    currentAnimFrame = (currentAnimFrame + 1) % frameCount;
  }
}

void set_open_menu(int menu) {
  current_menu = (menu % 2 + 2) % 2; // Correct negative modulo
  if (current_menu == 0) {
    drawMenuText("WIFI");
  } else {
    drawMenuText("Bright");
  }
}

void drawMenuBackground() {
  tft.drawXBitmap(0, 0, secondary_menu_bg_bits, 240, 240, TFT_BLACK, 0x8410);
  tft.drawXBitmap(0, 80, menu_background_bits, 240, 100, TFT_BLACK, TFT_WHITE);
}

void drawMenuText(const char* text) {
  tft.fillRect(main_item_boundaries.x, main_item_boundaries.y, main_item_boundaries.w, main_item_boundaries.h, TFT_BLACK);
  tft.setTextSize(3);
  tft.drawString(text, main_item_pos.x, main_item_pos.y);
}

void touch_loop() {
  bool touched = isTouched();

  if (touched && !wasTouched) {
    touchStartTime = millis();
    wasTouched = true;
    longPressDisplayed = false;
  }
  
  if (!touched && wasTouched) {
    unsigned long duration = millis() - touchStartTime;
    
    if (duration >= 700) {
      Serial.println("Long press released");
      if (current_menu == 0) {
      animate = true;
        drawMenuBackground();
        tft.setTextSize(1);
        tft.drawString("12:00", 40, 14);
        drawMenuText("WIFI");
      }
    } else {
      if (waitingForSecondTap) {
        set_open_menu(current_menu - 1);
        Serial.println("Double click detected");
        waitingForSecondTap = false;
        potentialSingleClick = false;
      } else {
        waitingForSecondTap = true;
        potentialSingleClick = true;
        lastTapTime = millis();
      }
    }
    wasTouched = false;
  }

  if (touched && wasTouched && !longPressDisplayed) {
    if (millis() - touchStartTime > 700) {
      Serial.println("Long press detected");
      if (current_menu == 0) {
        animate = false;
        tft.drawXBitmap(0, 0, wifi_qr_code, 240, 240, TFT_BLACK, TFT_WHITE);
      }
      longPressDisplayed = true;
    }
  }

  if (potentialSingleClick && millis() - lastTapTime > 300) {
    set_open_menu(current_menu + 1);
    Serial.println("Click detected");
    potentialSingleClick = false;
    waitingForSecondTap = false;
  }

  delay(20);
}

bool isTouched() {
  long total = 0;
  for (int i = 0; i < 5; i++) {
    total += readCapacitiveSensor(TOUCH_PIN);
  }
  long avg = total / 5;
  return avg < TOUCH_THRESHOLD;
}

long readCapacitiveSensor(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(1);

  pinMode(pin, INPUT);
  long start = micros();
  while (digitalRead(pin) == LOW) {
    if (micros() - start > 1000) break;
  }
  return micros() - start;
}
