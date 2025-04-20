#include "touch.h"

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
            if (current_menu == 0)
            {
                animate = true;
                drawMenuBackground();
                tft.setTextSize(1);
                tft.drawString("12:00", 40, 14);
                drawMenuText("WIFI");
            }
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
            if (current_menu == 0)
            {
                animate = false;
                tft.drawXBitmap(0, 0, wifi_qr_code, 240, 240, TFT_BLACK, TFT_WHITE);
            }
            longPressDisplayed = true;
        }
    }

    if (potentialSingleClick && millis() - lastTapTime > 300)
    {
        set_open_menu(current_menu + 1);
        Serial.println("Click detected");
        potentialSingleClick = false;
        waitingForSecondTap = false;
    }

    delay(20);
}