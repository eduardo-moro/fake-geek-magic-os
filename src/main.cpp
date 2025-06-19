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

unsigned long lastAnimTimeBat = 0;

// WiFi AP state
boolean ap_active = false;
boolean ap_connected = false;
unsigned long ap_active_time = 0;

// Web server
ESP8266WebServer server(80);
DNSServer dnsServer;

// Routing
String route = "menu";

// Timebox
int initial_timebox = 10;
int timebox = 0;
time_t last_timebox_update = 0;

void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TFT_BL, OUTPUT);

  analogWriteRange(255);
  analogWriteFreq(1000);

  setBrightnessPercent(60);

  Serial.begin(115200);
  Serial.println("Initializing TFT display...");

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  EEPROM.begin(EEPROM_SIZE);

  if (connectToBestNetwork())
  {
    Serial.println("Connected to saved network:");
    Serial.println(WiFi.SSID());

    configTime(TZ_OFFSET, TZ_DST, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);

    host_webpage();

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    Serial.printf("Current time in Brasília: %02d:%02d:%02d, %02d/%02d/%04d\n",
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                  timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  }

  initializeMenu();

  if (WiFi.status() == WL_CONNECTED)
  {
    tft.setTextSize(1);
    tft.drawString(WiFi.localIP().toString(), 120, 14);
  }
}

void loop()
{
  touch_loop();
  server.handleClient();

  if (route == "menu" && (millis() - lastUserActivity > 15000))
  {
    route = "clock";
    setBrightnessPercent(5);
    delay(500);
    start_clock();
  }

  if (route == "menu")
  {
    loop_battery();
    loop_icon();
    top_clock_loop();
  }
  else if (route == "clock")
  {
    clock_loop();
  }

  if (ap_active)
  {
    dnsServer.processNextRequest();

    qr_code_timeout();
  }
}