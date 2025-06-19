#include "animation.h"


void loop_icon()
{
    if (millis() - lastAnimTime >= icon_pos.anim_delay)
    {
        lastAnimTime = millis();
        tft.drawXBitmap(icon_pos.x, icon_pos.y, menu_items[current_menu].iconFrames[currentAnimFrame], icon_pos.w, icon_pos.h, TFT_BLACK, TFT_CYAN);
        currentAnimFrame = (currentAnimFrame + 1) % 4;
    }
}

void loop_wifi_icon()
{
    if (millis() - lastAnimTime >= icon_pos.anim_delay)
    {
        lastAnimTime = millis();
        tft.drawXBitmap(100, 100, menu_items[WIFI_MENU_INDEX].iconFrames[currentAnimFrame], icon_pos.w, icon_pos.h, TFT_BLACK, TFT_CYAN);
        currentAnimFrame = (currentAnimFrame + 1) % 4;
    }
}

void loop_battery()
{
    if (millis() - lastAnimTimeBat >= 500)
    {
        int level = getSignalStrengthLevel();
        
        static const unsigned char* battery_state_list[4] = {
            battery_icon_0,
            battery_icon_1,
            battery_icon_2,
            battery_icon_3,
        };

        static const unsigned int color_list[4] = {
            TFT_RED,
            TFT_ORANGE,
            TFT_CYAN,
            TFT_GREEN
        };
        
        lastAnimTimeBat = millis();
        tft.drawXBitmap(200, 4, battery_state_list[level], 20, 12, color_list[level], TFT_BLACK);
    }
}

