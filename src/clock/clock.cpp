#include "clock.h"

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
        Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        lastPrint = now;
    }
}

void drawTimeFullScreen()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char timeStr[9]; // "HH:MM:SS"
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(4);
    tft.drawString(timeStr, 120, 120);
}

void start_clock()
{
    tft.fillScreen(TFT_BLACK);
    drawTimeFullScreen();
}

void clock_loop() {
    static time_t lastPrint = 0;
    time_t now = time(nullptr);

    if (now != lastPrint) {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        drawTimeFullScreen();
        lastPrint = now;
    }
}