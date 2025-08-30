#ifndef PINS_H
#define PINS_H

#if defined(ESP8266)

#define BUTTON_PIN 4
#define TFT_BL 5

#elif defined(ESP32)

#define BUTTON_PIN 8
#define TFT_BL -1

#endif

#endif // PINS_H