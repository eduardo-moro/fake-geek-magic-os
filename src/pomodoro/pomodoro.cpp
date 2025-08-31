#include "pomodoro.hpp"

void drawPomodoroTime()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString(timeStr, 40, 14);
}

void top_pomodoro_clock_loop()
{
    static time_t lastPrint = 0;
    time_t now = time(nullptr);

    if (now - lastPrint >= 30)
    {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        drawPomodoroTime();
        Serial.printf("Time: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
        lastPrint = now;
    }
}

void drawPomodoroFullScreen()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    int minutes = pomodoro_c / 60;
    int seconds = pomodoro_c % 60;
    
    char timeStr[6]; // "HH:MM:SS"
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", minutes, seconds);

    tft.setTextSize(7);
    tft.setTextColor(pomodoroStatusColors[current_pomodoro], TFT_BLACK);
    tft.drawString(timeStr, 120, 120);
    tft.setTextColor(TFT_WHITE);

    if (pomodoro_c > 0)
    {
        time_t current = time(nullptr);
        if (current != last_pomodoro_update)
        {
            pomodoro_c--;
            if (pomodoro_c <= 0) {
                size_t length = 8;
                current_pomodoro++;
                current_pomodoro = current_pomodoro % length;
                pomodoro_c = pomodoro_times[current_pomodoro];
            }
            last_pomodoro_update = current;
        }

        int color = TFT_CYAN;

        if (pomodoro_c < 2 * 60)
        {
            color = TFT_RED;
        }
        else if (pomodoro_c < 5 * 60)
        {
            color = TFT_YELLOW;
        }
    }
}

void start_pomodoro()
{
    current_pomodoro = 0;
    pomodoro_c = pomodoro_times[current_pomodoro];
    tft.fillScreen(TFT_BLACK);
    drawPomodoroFullScreen();
}

void pomodoro_loop()
{
    static time_t lastPrint = 0;
    time_t now = time(nullptr);

    if (now != lastPrint)
    {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        drawPomodoroFullScreen();
        top_pomodoro_clock_loop();
        lastPrint = now;
    }
}