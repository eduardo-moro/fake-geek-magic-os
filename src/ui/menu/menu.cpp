#include "menu.hpp"

MenuItem menu_items[] = {
    {"RELOGIO", nextMenu, handleClockClick, prevMenu, {clock_0_bits, clock_1_bits, clock_2_bits, clock_3_bits}, 2, NULL},
    {"WIFI", nextMenu, handleWifiClick, prevMenu, {wifi_0_bits, wifi_1_bits, wifi_2_bits, wifi_3_bits}, 3, NULL},
    {"REDES", nextMenu, handleSelectWifiClick, prevMenu, {wifi_0_bits, wifi_1_bits, wifi_2_bits, wifi_3_bits}, 3, NULL},
    {"BRILHO", nextMenu, handleBrightClick, prevMenu, {bright_0_bits, bright_1_bits, bright_2_bits, bright_3_bits}, 2, NULL},
    {"TIMEBOX", nextMenu, handleTimeboxClick, prevMenu, {settings_0_bits, settings_1_bits, settings_2_bits, settings_3_bits}, 2, NULL},
    {"POMODORO", nextMenu, handlePomodoroClick, prevMenu, {pomodoro_0_bits, pomodoro_1_bits, pomodoro_2_bits, pomodoro_3_bits}, 2, NULL},
    {"ART", nextMenu, handleArtClick, prevMenu, {bright_0_bits, bright_1_bits, bright_2_bits, bright_3_bits}, 2, NULL},
    {"ANIMATE", nextMenu, handleAnimateClick, prevMenu, {bright_0_bits, bright_1_bits, bright_2_bits, bright_3_bits}, 2, NULL},
    {"TEST", nextMenu, handleTestMenuClick, prevMenu, {settings_0_bits, settings_1_bits, settings_2_bits, settings_3_bits}, 2, NULL}
};
#define MENU_COUNT 9

MenuItem original_menu_items[sizeof(menu_items) / sizeof(menu_items[0])];

void copy_menu_items()
{
    for (size_t i = 0; i < MENU_COUNT; ++i)
    {
        original_menu_items[i] = menu_items[i];
    }
}

void drawMenuBackground()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

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

void handleSelectWifiClick()
{
    // Use the new list menu system for WiFi selection
    showWifiSelectionMenu();
}

void returnToMenu()
{
    route = "menu";
    for (int i = 0; i < MENU_COUNT; ++i)
    {
        menu_items[i] = original_menu_items[i];
    }
    set_open_menu(WIFI_SELECT_INDEX);
}

void handleWifiSelected()
{
    for (int i = 0; i < MENU_COUNT; ++i)
    {
        menu_items[i] = original_menu_items[i];
    }

    connectToNetwork(menu_items[current_menu].label);
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


void initializeMenu()
{
    setBrightnessPercent(40);
    drawMenuBackground();
    drawTime();
    updateMenuLabels();
}

void handleTimeboxClick()
{
    initial_timebox = (initial_timebox + 5) % 60;
    timebox = initial_timebox * 60;

    static char timebox_label[10];
    snprintf(timebox_label, sizeof(timebox_label), "%d min", initial_timebox);
    menu_items[TIMEBOX_MENU_INDEX].label = timebox_label;

    tft.setTextSize(menu_items[current_menu].size);
    tft.fillRect(main_item_boundaries.x, main_item_boundaries.y, main_item_boundaries.w, main_item_boundaries.h, TFT_BLACK);
    tft.drawString(menu_items[current_menu].label, main_item_pos.x, main_item_pos.y);

    delay(500);

    menu_items[TIMEBOX_MENU_INDEX].label = "TIMEBOX";
    tft.setTextSize(menu_items[current_menu].size);
    tft.fillRect(main_item_boundaries.x, main_item_boundaries.y, main_item_boundaries.w, main_item_boundaries.h, TFT_BLACK);
    tft.drawString(menu_items[current_menu].label, main_item_pos.x, main_item_pos.y);

    tft.setTextSize(2);
}

void handlePomodoroClick() 
{
    route = "pomodoro";
    setBrightnessPercent(5);
    delay(500);
    start_pomodoro();
}

void handleBrightClick()
{
    static int brightness_level = 0;
    brightness_level = min(1, (brightness_level + 1) % 4);

    setBrightnessPercent(brightness_level * 20);

    static char label[10];
    snprintf(label, sizeof(label), "%d%%", brightness_level * 20);
    menu_items[BRIGHT_MENU_INDEX].label = label;

    tft.setTextSize(menu_items[current_menu].size);
    tft.fillRect(main_item_boundaries.x, main_item_boundaries.y, main_item_boundaries.w, main_item_boundaries.h, TFT_BLACK);
    tft.drawString(menu_items[current_menu].label, main_item_pos.x, main_item_pos.y);

    delay(500);

    menu_items[BRIGHT_MENU_INDEX].label = "BRILHO";
    tft.setTextSize(menu_items[current_menu].size);
    tft.fillRect(main_item_boundaries.x, main_item_boundaries.y, main_item_boundaries.w, main_item_boundaries.h, TFT_BLACK);
    tft.drawString(menu_items[current_menu].label, main_item_pos.x, main_item_pos.y);

    delay(500);
}

void handleTestMenuClick()
{
    // Show the test menu with 30 items
    showTestMenu();
}