#pragma once

#include "time.h"
#include "TFT_eSPI.h"

#define WORK 0
#define REST 1
#define LONG_WORK 2
#define LONG_REST 3

void start_pomodoro();
void pomodoro_loop();
void drawPomodoroFullScreen();
void top_pomodoro_clock_loop();
void drawPomodoroTime();

extern TFT_eSPI tft;
extern int pomodoro_c;
extern int pomodoro_times[];
extern int current_pomodoro;
extern time_t last_pomodoro_update;
extern uint16_t pomodoroStatusColors[];
