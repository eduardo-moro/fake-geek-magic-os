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
    {handleMenuClick, handleMenuPress, handleMenuDoubleClick, doNothing},   // MENU
    {handleClockQuit, doNothing, doNothing, doNothing},                     // CLOCK
    {handleClockQuit, doNothing, doNothing, doNothing},                     // POMODORO
    {handlePixelQuit, doNothing, doNothing, doNothing},                     // PIXEL
    {handlePixelQuit, doNothing, doNothing, doNothing},                     // ANIMATE
    {handleWifiQrQuit, doNothing, doNothing, doNothing},                    // WIFI_QR_CODE
    {handleListMenuClick, handleListMenuSelect, handleListMenuDoubleClick, doNothing},  // WIFI_SELECT
    {handleListMenuClick, handleListMenuSelect, handleListMenuDoubleClick, doNothing}   // TEST_MENU
#ifdef ESP32
    ,{handleBuddyAccept, handleBuddyQuit, doNothing, doNothing}              // BUDDY
#endif
};

std::map<String, int> routeMap = {
    {"menu", 0},
    {"clock", 1},
    {"pomodoro", 2},
    {"pixel", 3},
    {"animate", 4},
    {"wifi_qr_code", 5},
    {"wifi_select", 6},
    {"test_menu", 7}
#ifdef ESP32
    ,{"buddy", 8}
#endif
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
        Serial.print("Warning: Unknown route '");
        Serial.print(route);
        Serial.println("', defaulting to menu");
        current_route = 0; // Default to menu instead of -1
    }
}

bool isValidRoute() {
    return current_route >= 0 && current_route < (sizeof(commandHandler) / sizeof(commandHandler[0]));
}

void detectMenuTouch()
{
    #ifdef USE_TOUCH_SENSOR
    int reading = (touchRead(BUTTON_PIN) < TOUCH_THRESHOLD) ? 1 : 0;  // 1 = touched
    #else
    int reading = digitalRead(BUTTON_PIN);
    #endif

    // Button pressed when reading matches BTN_STTS
    if (reading == BTN_STTS)
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
            if (isValidRoute()) {
                commandHandler[current_route].onRelease();
            }
            registerUserActivity();
            Serial.println("released");
            digitalWrite(11, LOW);
        }
        else
        {
            if (waitingForSecondTap)
            {
                Serial.println("double");
                if (isValidRoute()) {
                    commandHandler[current_route].onDoubleClick();
                }
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
        if (isValidRoute()) {
            commandHandler[current_route].onLongPress();
        }
        registerUserActivity();
        longPressDisplayed = true;
    }

    if (potentialSingleClick && millis() - lastTapTime > 300)
    {
        Serial.println("click");
        if (isValidRoute()) {
            commandHandler[current_route].onClick();
        }
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

void handleWifiQrQuit()
{
    Serial.println("wifi qr quit");
    stop_ap();
    route = "menu";
    initializeMenu();
}

void handleListMenuClick()
{
    Serial.println("list menu click - next");
    listMenuNext();
}

void handleListMenuDoubleClick()
{
    Serial.println("list menu double click - prev");
    listMenuPrev();
}

void handleListMenuSelect()
{
    Serial.println("list menu select");
    listMenuSelect();
}

#ifdef ESP32
void handleBuddyAccept()
{
    // Button A pressed (pin 32, cap-touch) = approve permission prompt
    Serial.println("buddy: button A pressed (approve)");
    buddy_app_send_approve();
}

void handleBuddyQuit()
{
    // Long press button A (hold) = exit buddy app
    Serial.println("buddy: button A held (exit)");
    buddy_app_quit();
    route = "menu";
    initializeMenu();
}
#endif

void doNothing() {}

void registerUserActivity()
{
    lastUserActivity = millis();
}