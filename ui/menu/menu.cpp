#include "menu.h"

void drawMenuBackground()
{
    tft.drawXBitmap(0, 0, secondary_menu_bg_bits, 240, 240, TFT_BLACK, 0x8410);
    tft.drawXBitmap(0, 80, menu_background_bits, 240, 100, TFT_BLACK, TFT_WHITE);
}

void drawMenuText(const char *text)
{
    tft.fillRect(main_item_boundaries.x, main_item_boundaries.y, main_item_boundaries.w, main_item_boundaries.h, TFT_BLACK);
    tft.setTextSize(3);
    tft.drawString(text, main_item_pos.x, main_item_pos.y);
}

void set_open_menu(int menu)
{
    current_menu = (menu % 2 + 2) % 2;
    if (current_menu == 0)
    {
        drawMenuText("WIFI");
    }
    else
    {
        drawMenuText("Bright");
    }
}
