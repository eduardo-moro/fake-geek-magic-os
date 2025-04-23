#include "animation.h"

extern TFT_eSPI tft;
extern unsigned long lastAnimTime;
extern unsigned long lastAnimTimeBat;
extern int currentAnimFrame;
extern int current_menu;
extern int currentAnimFrameBat;

void loop_icon()
{
    if (millis() - lastAnimTime >= icon_pos.anim_delay)
    {
        lastAnimTime = millis();
        tft.drawXBitmap(icon_pos.x, icon_pos.y, menu_items[current_menu].iconFrames[currentAnimFrame], icon_pos.w, icon_pos.h, TFT_BLACK, TFT_CYAN);
        currentAnimFrame = (currentAnimFrame + 1) % 4;
    }
}

void loop_battery()
{
    if (millis() - lastAnimTimeBat >= 250)
    {
        lastAnimTimeBat = millis();
        tft.drawXBitmap(200, 9, battery_state_list[currentAnimFrameBat], 20, 12, TFT_WHITE, TFT_BLACK);
        currentAnimFrameBat = (currentAnimFrameBat + 1) % 6;
    }
}

String zeroPad(int num) {
    if (num < 10) return "00" + String(num);
    else if (num < 100) return "0" + String(num);
    else return String(num);
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
    tft.pushImage(x, y, w, h, bitmap);
    return true;
}

void animate_splash_screen() {
    const int FRAME_COUNT = 25;
    const long FRAME_DELAY_MS = 1000 / 30; // ~33ms per frame (30 FPS)
    unsigned long frameStartTime;
    long elapsedTime;

    for (int i = 0; i < FRAME_COUNT; ++i) {
        frameStartTime = millis();
        String path = "/splash_" + zeroPad(i) + "_baseline.jpg";
        
        if (!SPIFFS.exists(path)) {
            Serial.println("Missing file: " + path);
            continue; // Skip missing frames
        }

        Serial.println("Drawing: " + path);
        int result = TJpgDec.drawFsJpg(0, 0, path.c_str());
        
        if (result == 0) {
            Serial.println("Error drawing image: " + path);
        } else {
            Serial.println("Successfully drawn image: " + path);
        }

        // Calculate remaining time to maintain frame rate
        elapsedTime = millis() - frameStartTime;
        if (elapsedTime < FRAME_DELAY_MS) {
            delay(FRAME_DELAY_MS - elapsedTime);
        } else {
            Serial.println("Frame rendering too slow; consider optimizing images.");
        }
    }
}