#include "main.hpp"

const int PWM_CHANNEL = 0;

TFT_eSPI tft = TFT_eSPI();

// Mqtt connection
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMqttAttempt = 0;
const unsigned long mqttReconnectInterval = 500;

// State definitions
int current_menu = 0;
int valid_menu_items = 0;

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
bool ap_active = false;
bool ap_connected = false;
unsigned long ap_active_time = 0;

// Web server
#if defined(ESP8266)
ESP8266WebServer server(80);
#elif defined(ESP32)
WebServer server(80);
#endif
DNSServer dnsServer;

// Routing
String route = "menu";
String previous_route = "";

// Timebox
int initial_timebox = 10;
int timebox = 0;
time_t last_timebox_update = 0;

// Pomodoro
const int pomodoro_times[] = {
  25 * 60, 5 * 60, // work, rest
  25 * 60, 5 * 60, // work, rest
  25 * 60, 5 * 60, // work, rest
  45 * 60, 15 * 60 // long work, long rest
};

int current_pomodoro = 0;
int pomodoro_c = pomodoro_times[current_pomodoro];
time_t last_pomodoro_update = 0;
uint16_t const pomodoroStatusColors[] = {
    TFT_ORANGE,
    TFT_CYAN, 
    TFT_ORANGE,
    TFT_CYAN, 
    TFT_ORANGE,
    TFT_CYAN, 
    TFT_RED,
    TFT_GREEN
};

void handleArtClick() {
    start_pixel_art();
}

void handleAnimateClick() {
    start_animate();
}

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

  EEPROM.begin(EEPROM_SIZE);

  if (connectToBestNetwork())
  {
    // Serial.println("Connected to saved network:");
    // Serial.println(WiFi.SSID());

    host_webpage();

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    // Serial.printf("Current time in Brasília: %02d:%02d:%02d, %02d/%02d/%04d\n",
    //               timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
    //               timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  initializeMenu();

  if (WiFi.status() == WL_CONNECTED)
  {
    tft.setTextSize(1);
    tft.drawString(WiFi.localIP().toString(), 120, 14);
    
    setup_MQTT();
  }
}

void loop()
{
  if (currentNotification.hasUnread) {
    displayNotification();
    if (digitalRead(BUTTON_PIN) == HIGH) { // button is touched
        currentNotification.hasUnread = false;
        tft.fillScreen(TFT_BLACK);
        delay(100); // simple debounce
    }
    return;
  }

  touch_loop();
  server.handleClient();

  if (WiFi.status() == WL_CONNECTED || ap_active) {
    if (!client.connected()) {
      attempt_MQTT_reconnect();
    }
    
    if (!client.loop()) {
      // Serial.println("client.loop() returned false");
    }
  }

  if (route == "pixel" && previous_route == "pomodoro") {
    pomodoro_background_handler();
  }

  if (route == "menu" && (millis() - lastUserActivity > 15000))
  {
    /* to use clock as main*/
    /*route = "clock";
    setBrightnessPercent(5);
    delay(500);
    start_clock();*/

    route = "pomodoro";
    setBrightnessPercent(5);
    delay(500);
    start_pomodoro();
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
  else if (route == "pomodoro")
  {
    pomodoro_loop();
  }
  else if (route == "pixel")
  {
    pixels_loop();
  }
  else if (route == "animate")
  {
    animate_loop();
  }

  if (ap_active)
  {
    dnsServer.processNextRequest();

    qr_code_timeout();
  }
}
