#ifndef CLOCK_H
#define CLOCK_H

#include <time.h>
#include <TFT_eSPI.h>

// Brasília Timezone (UTC-3, no DST)
#define TZ_OFFSET -3 * 3600 // in seconds
#define TZ_DST 0            // no DST

void top_clock_loop();
void start_clock();
void clock_loop();
void drawTime();

extern TFT_eSPI tft;
extern int timebox;
extern int initial_timebox;
extern time_t last_timebox_update;

#endif