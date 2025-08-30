#include "clock.hpp"

void drawTime()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char timeStr[6]; // "HH:MM"
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString(timeStr, 40, 14);
}

void top_clock_loop()
{
    static time_t lastPrint = 0;
    time_t now = time(nullptr);

    if (now - lastPrint >= 30)
    {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        drawTime();
        Serial.printf("Time: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
        lastPrint = now;
    }
}

void drawTimeFullScreen()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    char timeStr[6]; // "HH:MM:SS"
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    tft.setTextSize(7);
    tft.drawString(timeStr, 120, 120);

    int secw = map(timeinfo.tm_sec, 0, 60, 0, 240);

    tft.setTextSize(2);

    tft.fillRect(0, 238, secw, 240, TFT_CYAN);
    tft.fillRect(secw, 238, 240 - secw, 2, TFT_BLACK);

    // timebox countdown
    if (timebox > 0)
    {
        time_t current = time(nullptr);
        if (current != last_timebox_update && timebox > 0)
        {
            timebox--;
            if (timebox <= 0)
            {
                initial_timebox = 10;
            }
            last_timebox_update = current;
        }

        secw = map(timebox, 0, initial_timebox * 60, 0, 240);

        int color = TFT_CYAN;

        if (timebox < 2 * 60)
        {
            color = TFT_RED;
        }
        else if (timebox < 5 * 60)
        {
            color = TFT_YELLOW;
        }

        int h = 233;
        tft.fillRect(0, h, secw, 3, color);
        tft.fillRect(secw, h, 240 - secw, 3, TFT_BLACK);
    }
}

void start_clock()
{
    tft.fillScreen(TFT_BLACK);
    drawTimeFullScreen();
}

void clock_loop()
{
    static time_t lastPrint = 0;
    time_t now = time(nullptr);

    if (now != lastPrint)
    {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        drawTimeFullScreen();
        lastPrint = now;
    }
}