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

    #if defined(ESP8266)
    tft.setTextSize(7);
    #elif  defined(ESP32C3)
    tft.setTextSize(4);
    #elif  defined(ESP32)
    tft.setTextSize(7);
    #endif
    tft.drawString(timeStr, TFT_WIDTH/2, TFT_HEIGHT/2);

    int secw = map(timeinfo.tm_sec, 0, 60, 0, TFT_WIDTH);

    tft.setTextSize(2);

    tft.fillRect(0, TFT_HEIGHT-2, secw, TFT_WIDTH, TFT_CYAN);
    tft.fillRect(secw, TFT_HEIGHT-2, TFT_WIDTH - secw, 2, TFT_BLACK);

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

        secw = map(timebox, 0, initial_timebox * 60, 0, TFT_WIDTH);

        int color = TFT_CYAN;

        if (timebox < 2 * 60)
        {
            color = TFT_RED;
        }
        else if (timebox < 5 * 60)
        {
            color = TFT_YELLOW;
        }

        int h = TFT_WIDTH -7;
        tft.fillRect(0, h, secw, 3, color);
        tft.fillRect(secw, h, TFT_WIDTH - secw, 3, TFT_BLACK);
    }
}

void start_clock()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
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