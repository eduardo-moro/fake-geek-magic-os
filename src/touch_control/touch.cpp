#include "touch.hpp"

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
    {handleMenuClick, handleMenuPress, handleMenuDoubleClick, doNothing},   // WIFI
    {handleClockQuit, doNothing, doNothing, doNothing},                     // RELOGIO
    {handleClockQuit, doNothing, doNothing, doNothing},                     // POMODORO
    {handlePixelQuit, doNothing, doNothing, doNothing},                     // PIXEL
    {handlePixelQuit, doNothing, doNothing, doNothing}                      // ANIMATE
};

std::map<String, int> routeMap = {
    {"menu", 0},
    {"clock", 1},
    {"pomodoro", 2},
    {"pixel", 3},
    {"animate", 4}
};

void touch_loop()
{
    updateRoute(route);
    detectMenuTouch();
}

void updateRoute(const String& route) {
    auto it = routeMap.find(route);
    if (it != routeMap.end()) {
        current_route = it->second;
    } else {
        current_route = -1;
    }
}

void detectMenuTouch()
{
    int reading = digitalRead(BUTTON_PIN);

    if (reading == LOW)
    {
        if (buttonState == BTN_RELEASED)
        {
            lastDebounceTime = millis();
            buttonState = BTN_DEBOUNCING;
        }
    }
    else
    {
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
        digitalWrite(11, HIGH);
    }

    if (wasTouched && buttonState == BTN_RELEASED)
    {
        unsigned long duration = millis() - touchStartTime;

        if (duration >= 700)
        {
            commandHandler[current_route].onRelease();
            registerUserActivity();
            Serial.println("released");
            digitalWrite(11, LOW);
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

void handlePixelQuit()
{
    Serial.println("pixel click");
    route = "menu";
    is_displaying_image = false;
    initializeMenu();
}

void doNothing() {}

void registerUserActivity()
{
    lastUserActivity = millis();
}