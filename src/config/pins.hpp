#ifndef PINS_H
#define PINS_H

#if defined(ESP8266)

#define BUTTON_PIN 4
#define TFT_BL 5
#define BTN_STTS HIGH

#elif  defined(ESP32C3)

#define BUTTON_PIN 10
#define TFT_BL 11
#define BTN_STTS LOW

#elif  defined(ESP32)

#define BUTTON_PIN 32   // GPIO 32 - Capacitive touch sensor (T9)
#define TFT_BL 25       // GPIO 25 - PWM_i1 (backlight)
#define BTN_STTS HIGH
#define USE_TOUCH_SENSOR 1  // Enable capacitive touch API
#define TOUCH_THRESHOLD 75  // Lower = more sensitive (untouched ~60-80, touched ~20-40)

#endif

#endif // PINS_H