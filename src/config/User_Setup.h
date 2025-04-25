
// Custom User Setup for Geek Magic Clock (ESP8266 + ST7789)

#define ST7789_DRIVER

// Display size
#define TFT_WIDTH 240
#define TFT_HEIGHT 240  // or 320 depending on your screen

// Define control pins
#define TFT_CS   -1  // Not used
#define TFT_DC    0
#define TFT_RST   2

#define TFT_BL    5
#define TFT_BACKLIGHT_ON LOW

// SPI pins for ESP8266
#define TFT_MOSI 13
#define TFT_SCLK 14
// MISO not used on ST7789

// Color order (try commenting this if colors are off)
#define TFT_RGB_ORDER TFT_BGR

// Load fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT
