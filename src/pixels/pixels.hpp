#ifndef PIXELS_H
#define PIXELS_H

#include "Arduino.h"

extern bool is_displaying_image;

typedef struct {
    char data[1024]; 
    bool received_parts[8];
    int image_id;
} ImageReassemblyBuffer;

extern ImageReassemblyBuffer current_image_buffer;

void pixels_loop();
void start_pixel_art();
void set_pixel_data(const char *data);
void animate_loop();
void start_animate();
void process_image_part(int image_id, int part_number, const char* part_data);

#endif