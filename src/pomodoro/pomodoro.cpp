#include "pomodoro.hpp"

void drawPomodoroTime()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.drawString(timeStr, 120, 66);
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

    Serial.print("DEBUG: Inside colorToString - angle: ");
    Serial.print(angle);
    Serial.print(", r: ");
    Serial.print(r);
    Serial.print(", g: ");
    Serial.print(g);
    Serial.print(", b: ");
    Serial.println(b);

    return String(angle) + "," + String(r) + "," + String(g) + "," + String(b);
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

    int limit = pomodoro_times[current_pomodoro];
    int angle = 361 - map(pomodoro_c, 0, limit, 0, 360);

    client.publish(MQTT_TOPIC_COMMAND, colorToString(angle, pomodoroStatusColors[current_pomodoro]).c_str());
    tft.drawArc(120, 120, 100, 96, 0, angle, pomodoroStatusColors[current_pomodoro], TFT_BLACK, true);

    tft.setTextSize(5);
    tft.drawString(timeStr, 120, 120);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    time_t current = time(nullptr);
    if (current != last_pomodoro_update)
    {
        if (pomodoro_c <= 0) {
            size_t length = 8;
            current_pomodoro++;
            current_pomodoro = current_pomodoro % length;
            pomodoro_c = pomodoro_times[current_pomodoro];
            tft.drawArc(120, 120, 100, 96, 0, angle, TFT_BLACK, TFT_BLACK, true);
            client.publish(MQTT_TOPIC_COMMAND, colorToString(360, TFT_BLACK).c_str());
            delay(200);
            tft.drawArc(120, 120, 100, 96, 0, angle, pomodoroStatusColors[current_pomodoro], TFT_BLACK, true);
            client.publish(MQTT_TOPIC_COMMAND, colorToString(angle, pomodoroStatusColors[current_pomodoro]).c_str());
            delay(400);
            tft.drawArc(120, 120, 100, 96, 0, angle, TFT_BLACK, TFT_BLACK, true);
            client.publish(MQTT_TOPIC_COMMAND, colorToString(360, TFT_BLACK).c_str());
            delay(200);
            tft.drawArc(120, 120, 100, 96, 0, angle, pomodoroStatusColors[current_pomodoro], TFT_BLACK, true);
            client.publish(MQTT_TOPIC_COMMAND, colorToString(angle, pomodoroStatusColors[current_pomodoro]).c_str());
            delay(400);
            tft.drawArc(120, 120, 100, 96, 0, angle, TFT_BLACK, TFT_BLACK, true);
            client.publish(MQTT_TOPIC_COMMAND, colorToString(360, TFT_BLACK).c_str());
            delay(200);
            tft.drawArc(120, 120, 100, 96, 0, angle, pomodoroStatusColors[current_pomodoro], TFT_BLACK, true);
            client.publish(MQTT_TOPIC_COMMAND, colorToString(angle, pomodoroStatusColors[current_pomodoro]).c_str());
            delay(400);
            tft.drawArc(120, 120, 100, 96, 0, angle, TFT_BLACK, TFT_BLACK, true);
            client.publish(MQTT_TOPIC_COMMAND, colorToString(360, TFT_BLACK).c_str());
        }
        last_pomodoro_update = current;
        pomodoro_c--;
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