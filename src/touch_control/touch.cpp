#include "touch.h"

extern bool wasTouched;
extern bool longPressDisplayed;
extern bool waitingForSecondTap;
extern bool potentialSingleClick;
extern unsigned long lastTapTime;
extern unsigned long touchStartTime;
extern int current_menu;
extern bool animate;
extern TFT_eSPI tft;
static unsigned long lastTouchCheck = 0;

bool isTouched()
{
    long total = 0;
    for (int i = 0; i < 5; i++)
    {
        total += readCapacitiveSensor(TOUCH_PIN);
    }
    long avg = total / 5;
    return avg < TOUCH_THRESHOLD;
}

long readCapacitiveSensor(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(1);

    pinMode(pin, INPUT);
    long start = micros();
    while (digitalRead(pin) == LOW)
    {
        if (micros() - start > 1000)
            break;
    }
    return micros() - start;
}

void touch_loop()
{
    if (millis() - lastTouchCheck < 20) return;
    lastTouchCheck = millis();

    bool touched = isTouched();

    if (touched && !wasTouched)
    {
        touchStartTime = millis();
        wasTouched = true;
        longPressDisplayed = false;
    }

    if (!touched && wasTouched)
    {
        unsigned long duration = millis() - touchStartTime;

        if (duration >= 700)
        {
            Serial.println("Long press released");
        }
        else
        {
            if (waitingForSecondTap)
            {
                set_open_menu(current_menu - 1);
                Serial.println("Double click detected");
                waitingForSecondTap = false;
                potentialSingleClick = false;
            }
            else
            {
                waitingForSecondTap = true;
                potentialSingleClick = true;
                lastTapTime = millis();
            }
        }
        wasTouched = false;
    }

    if (touched && wasTouched && !longPressDisplayed)
    {
        if (millis() - touchStartTime > 700)
        {
            Serial.println("Long press detected");
            menu_items[current_menu].onLongPress();
            longPressDisplayed = true;
        }
    }

    if (potentialSingleClick && millis() - lastTapTime > 300)
    {
        menu_items[current_menu].onClick();
        Serial.println("Click detected");
        potentialSingleClick = false;
        waitingForSecondTap = false;
    }
}

