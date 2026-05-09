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

// SPI pins for ESP8266
#define TFT_MOSI 13
#define TFT_SCLK 14
// MISO not used on ST7789

#define TFT_BACKLIGHT_ON LOW

#elif  defined(ESP32C3)

#define ST7735_DRIVER      // Define additional parameters below for this display

#define TFT_RGB_ORDER TFT_RGB  // Colour order Blue-Green-Red
#define TFT_INVERSION_ON


#define TFT_WIDTH  128
#define TFT_HEIGHT 128

#define ST7735_GREENTAB3

#define TFT_MOSI  4   // 1.90 4  1.44  4
#define TFT_SCLK  3 // 1.90 11   1.44  3

#define TFT_CS    2  // Chip select control pin  1.90 12  1.44 2
#define TFT_DC    0 // Data Command control pin   1.90 7  1.44 0
#define TFT_RST   5  // Reset pin (could connect to RST pin)  1.99 13  1.44 5

#define SPI_FREQUENCY  10000000
#define SPI_READ_FREQUENCY  6000000

#define TFT_BACKLIGHT_ON HIGH

#elif  defined(ESP32)

#define ST7789_DRIVER

// Display size
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define TFT_RGB_ORDER TFT_BGR

// SPI pins for ESP32
#define TFT_MOSI  23   // GPIO 23 - SPI MOSI1
#define TFT_SCLK  18   // GPIO 18 - SPI CLK1
#define TFT_CS    3    // GPIO 3 - SPI CS1
#define TFT_DC    2    // GPIO 2 - SPI DC1
#define TFT_RST   4    // GPIO 4 - Display Rst
#define TFT_BL    25   // GPIO 25 - PWM backlight

#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define TFT_BACKLIGHT_ON HIGH

#endif

// ============== TFT Common ===================


// Load fonts
#define LOAD_GLCD
#define LOAD_FONT2
//#define LOAD_FONT4
//#define LOAD_FONT6
//#define LOAD_FONT7
//#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define TOUCH_CS -1
