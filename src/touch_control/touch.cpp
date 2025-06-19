#include "touch.h"

static unsigned long lastTouchCheck = 0;
int current_route = 0;
const int DEBOUNCE_DELAY = 50;

enum ButtonState
{
    BTN_RELEASED,
    BTN_PRESSED,
    BTN_DEBOUNCING
};

ButtonState buttonState = BTN_RELEASED;
unsigned long lastDebounceTime = 0;
unsigned long lastUserActivity = 0;

MenuCommand commandHandler[] = {
    {handleMenuClick, handleMenuPress, handleMenuDoubleClick, doNothing}, // WIFI
    {handleClockQuit, doNothing, doNothing, doNothing}                    // RELOGIO
};

void touch_loop()
{
    if (route == "menu")
    {
        current_route = 0;
    }
    else if (route == "clock")
    {
        current_route = 1;
    }
    detectMenuTouch();
}

void detectMenuTouch()
{
    int reading = digitalRead(BUTTON_PIN);
    Serial.println(reading);

    // Added debounce logic
    if (reading == LOW)
    { // Button pressed
        if (buttonState == BTN_RELEASED)
        {
            lastDebounceTime = millis();
            buttonState = BTN_DEBOUNCING;
        }
    }
    else
    { // Button released
        buttonState = BTN_RELEASED;
    }

    if (buttonState == BTN_DEBOUNCING && (millis() - lastDebounceTime) > DEBOUNCE_DELAY)
    {
        buttonState = BTN_PRESSED;
        touchStartTime = millis();
        wasTouched = true;
        longPressDisplayed = false;
    }

    if (!wasTouched && buttonState == BTN_PRESSED)
    {
        wasTouched = true;
    }

    if (wasTouched && buttonState == BTN_RELEASED)
    {
        unsigned long duration = millis() - touchStartTime;

        if (duration >= 700)
        {
            commandHandler[current_route].onRelease();
            registerUserActivity();
            Serial.println("released");
        }
        else
        {
            if (waitingForSecondTap)
            {
                Serial.println("double");
                commandHandler[current_route].onDoubleClick();
                registerUserActivity();
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
        buttonState = BTN_RELEASED;
    }

    if (wasTouched && (millis() - touchStartTime > 700) && !longPressDisplayed)
    {
        Serial.println("long");
        commandHandler[current_route].onLongPress();
        registerUserActivity();
        longPressDisplayed = true;
    }

    if (potentialSingleClick && millis() - lastTapTime > 300)
    {
        Serial.println("click");
        commandHandler[current_route].onClick();
        registerUserActivity();
        potentialSingleClick = false;
        waitingForSecondTap = false;
    }
}

void handleMenuClick()
{
    menu_items[current_menu].onClick();
}

void handleMenuPress()
{
    menu_items[current_menu].onLongPress();
}

void handleMenuDoubleClick()
{
    menu_items[current_menu].onDoubleClick();
}

void handleMenuRelease()
{
}

void handleClockQuit()
{
    Serial.println("clock click");
    route = "menu";
    initializeMenu();
}

void registerUserActivity()
{
    lastUserActivity = millis();
}