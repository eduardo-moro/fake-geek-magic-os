#include "menu.h"

MenuItem menu_items[] = {
    {"RELOGIO", nextMenu, handleClockClick, prevMenu, {clock_0_bits, clock_1_bits, clock_2_bits, clock_3_bits}, 2, NULL},
    {"WIFI", nextMenu, handleWifiClick, prevMenu, {wifi_0_bits, wifi_1_bits, wifi_2_bits, wifi_3_bits}, 3, NULL},
    {"BRILHO", nextMenu, doNothing, prevMenu, {bright_0_bits, bright_1_bits, bright_2_bits, bright_3_bits}, 2, NULL},
    {"TIMEBOX", nextMenu, doNothing, prevMenu, {settings_0_bits, settings_1_bits, settings_2_bits, settings_3_bits}, 2, NULL},
};

void drawMenuBackground()
{
    tft.drawXBitmap(0, 0, secondary_menu_bg_bits, 240, 240, TFT_BLACK, 0x8410);
    tft.drawXBitmap(0, 80, menu_background_bits, 240, 100, TFT_BLACK, TFT_WHITE);
}

void updateMenuLabels()
{
    // main item
    tft.setTextSize(menu_items[current_menu].size);
    tft.fillRect(main_item_boundaries.x, main_item_boundaries.y, main_item_boundaries.w, main_item_boundaries.h, TFT_BLACK);
    tft.drawString(menu_items[current_menu].label, main_item_pos.x, main_item_pos.y);

    tft.setTextSize(2);

    // top item
    int num_items = sizeof(menu_items) / sizeof(menu_items[0]);
    int prev_menu = ((current_menu - 1) % num_items + num_items) % num_items;

    tft.fillRect(top_item_pos.x, top_item_pos.y, top_item_pos.w, top_item_pos.h, TFT_BLACK);
    tft.drawString(menu_items[prev_menu].label, top_item_pos.x + top_item_pos.w / 2, top_item_pos.y + top_item_pos.h / 2);

    // bottom item
    int next_menu = (current_menu + 1) % num_items;
    tft.fillRect(bottom_item_pos.x, bottom_item_pos.y, bottom_item_pos.w, bottom_item_pos.h, TFT_BLACK);
    tft.drawString(menu_items[next_menu].label, bottom_item_pos.x + bottom_item_pos.w / 2, bottom_item_pos.y + bottom_item_pos.h / 2);
}

void nextMenu()
{
    set_open_menu(current_menu + 1);
}

void prevMenu()
{
    set_open_menu(current_menu - 1);
}

void handleWifiClick()
{
    route = "wifi_qr_code";
    tft.drawXBitmap(0, 0, wifi_qr_code, 240, 240, TFT_BLACK, TFT_WHITE);
    setBrightnessPercent(5);
    start_ap();
}

void handleClockClick()
{
    route = "clock";
    setBrightnessPercent(5);
    delay(500);
    start_clock();
}

void set_open_menu(int menu)
{
    int num_items = sizeof(menu_items) / sizeof(menu_items[0]);
    current_menu = ((menu % num_items) + num_items) % num_items;

    updateMenuLabels();
}

void doNothing()
{
}

void initializeMenu()
{
    setBrightnessPercent(40);
    drawMenuBackground();
    drawTime();
    updateMenuLabels();
}

void handleTimeboxClick()
{
    timebox = initial_timebox * 60;
}