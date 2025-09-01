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
    tft.drawString(timeStr, 120, 62);
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

String colorToString(int angle, uint16_t color565) {
    uint8_t r = ((color565 >> 11) & 0x1F) * 255 / 31;
    uint8_t g = ((color565 >> 5) & 0x3F) * 255 / 63;
    uint8_t b = (color565 & 0x1F) * 255 / 31;

    return String(angle) + String(r) + "," + String(g) + "," + String(b);
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

    int angle = map(pomodoro_times[current_pomodoro] - pomodoro_c, 0, pomodoro_times[current_pomodoro], 0, 360);
    client.publish(MQTT_TOPIC_COMMAND, colorToString(angle, pomodoroStatusColors[current_pomodoro]).c_str());
    
    tft.setTextSize(5);
    tft.drawArc(120, 120, 100, 96, 0, angle, pomodoroStatusColors[current_pomodoro], TFT_BLACK, true);
    tft.drawString(timeStr, 120, 120);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

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