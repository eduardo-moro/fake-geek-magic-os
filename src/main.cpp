#include "main.h"

const int PWM_CHANNEL = 0;

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

unsigned long lastAnimTimeBat = 0;
int currentAnimFrameBat = 0;

// WiFi AP state
boolean ap_active = false;
boolean ap_connected = false;
unsigned long ap_active_time = 0;

char* current_route = "menu";

void setup()
{
  pinMode(BACKLIGHT_PIN, OUTPUT);

  analogWriteRange(255);
  analogWriteFreq(1000);

  setBrightnessPercent(60);

  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);

  Serial.println("Initializing TFT display...");

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  // splash screen
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS mount failed!");
  } else {
    Serial.println("SPIFFS mount successful.");
  }
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  animate_splash_screen();

  drawMenuBackground();
  tft.setTextSize(1);
  tft.drawString("12:00", 40, 14);
  updateMenuLabels();
}

void loop()
{

  touch_loop();

  if (animate)
  {
    loop_battery();
    loop_icon();
  }

  if (ap_active)
  {
    ap_connected = WiFi.status() == WL_CONNECTED;
    if (ap_connected)
    {
      stop_ap();
      ap_active = false;
    }

    if ((millis() - ap_active_time > 10000 || ap_connected) && current_route != "menu")
    {
      // TODO: move logic to a initialize_menu function
      current_route = "menu";
      animate = true;
      setBrightnessPercent(60);
      drawMenuBackground();
      tft.setTextSize(1);
      tft.drawString("12:00", 40, 14);
      tft.drawString("ap on", 120, 14);
      updateMenuLabels();
    }
  }
}