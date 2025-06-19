

// Custom User Setup for Geek Magic Clock (ESP8266 + ST7789)

#if defined(ESP8266)

#define ST7789_DRIVER

// Display size
#define TFT_WIDTH 240
#define TFT_HEIGHT 240

// Define control pins
#define TFT_CS -1 // Not used
#define TFT_DC 0
#define TFT_RST 2

#define TFT_BL 5

// SPI pins for ESP8266
#define TFT_MOSI 13
#define TFT_SCLK 14
// MISO not used on ST7789

#elif defined(ESP32)

#define ST7735_DRIVER
#define TFT_RGB_ORDER TFT_BGR

// Display size
#define TFT_WIDTH 128
#define TFT_HEIGHT 128

// Define control pins
#define TFT_CS 2
#define TFT_DC 0
#define TFT_RST 5

#define TFT_BL -1 // Backlight pin not used

// SPI pins for ESP8266
#define TFT_MOSI 4
#define TFT_SCLK 3

#define ST7735_GREENTAB3

#define TFT_INVERSION_ON

#define SPI_FREQUENCY 40000000      // 40 MHz
#define SPI_READ_FREQUENCY 20000000 // 20 MHz
#define SPI_TOUCH_FREQUENCY 2500000 // 2.5 MHz

#endif // ESP32

// ============== TFT Common ===================

#define TFT_BACKLIGHT_ON LOW

// Load fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

// Color order (try commenting this if colors are off)
#define TFT_RGB_ORDER TFT_BGR

#define TOUCH_CS -1