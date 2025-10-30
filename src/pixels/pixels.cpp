#include "pixels.hpp"
#include "display/display.hpp"
#include "main.hpp"

extern TFT_eSPI tft;

uint8_t pixel_data[32][32];
bool is_displaying_image = false;
unsigned long art_display_start_time = 0;

uint8_t image_history[10][32][32];
int history_index = 0;
int images_in_history = 0;

ImageReassemblyBuffer current_image_buffer;

const uint16_t color_map[] = {
    TFT_BLACK,                  // 0 #000000 - black (outline, contrast)
    tft.color565(228, 59, 68),  // 1 #e43b44 - strong red
    tft.color565(255, 163, 0),  // 2 #ffa300 - vibrant orange
    tft.color565(251, 242, 54), // 3 #fbf236 - bright yellow
    tft.color565(50, 205, 50),  // 4 #32cd32 - saturated green
    tft.color565(0, 255, 255),  // 5 #00ffff - cyan/teal
    tft.color565(36, 114, 200), // 6 #2472c8 - strong blue
    tft.color565(123, 65, 217), // 7 #7b41d9 - purple
    tft.color565(248, 24, 148), // 8 #f81894 - pink
    TFT_WHITE                   // 9 #FFFFFF - white (highlight, background)
};

void set_pixel_data(const char *data) {
    Serial.println("set_pixel_data called");
    int data_len = strlen(data);
    Serial.print("Data length: ");
    Serial.println(data_len);
    for (int i = 0; i < 1024; ++i) {
        int row = i / 32;
        int col = i % 32;
        uint8_t value;
        if (i < data_len) {
            value = data[i] - '0';
        } else {
            value = 0; // Fill with black
        }
        pixel_data[row][col] = value;
        image_history[history_index][row][col] = value;
    }
    history_index = (history_index + 1) % 10;
    if (images_in_history < 10) {
        images_in_history++;
    }
    Serial.println("set_pixel_data finished");
}

void start_pixel_art() {
    Serial.println("start_pixel_art called");
    previous_route = route;
    route = "pixel";
    is_displaying_image = true;
    art_display_start_time = millis();
    tft.fillScreen(TFT_BLACK);
}

void pixels_loop() {
    if (!is_displaying_image) return;

    if (millis() - art_display_start_time > 30000) {
        Serial.println("30 second timeout reached. Returning to previous route.");
        route = previous_route;
        is_displaying_image = false;
        if (route == "pomodoro") {
            tft.fillScreen(TFT_BLACK);
        } else {
            route = "menu";
            initializeMenu();
        }
        return;
    }

    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            uint8_t color_index = pixel_data[y][x];
            if (color_index >= 0 && color_index <= 9) {
                tft.fillRect(8 + x * 7, 8 + y * 7, 7, 7, color_map[color_index]);
            }
        }
    }
}

void start_animate() {
    if (images_in_history == 0) {
        Serial.println("No images in history to animate.");
        return;
    }
    route = "animate";
    tft.fillScreen(TFT_BLACK);
}

void animate_loop() {
    static int current_frame = 0;
    static unsigned long lastAnimTime = 0;

    if (millis() - lastAnimTime > 100) {
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                uint8_t color_index = image_history[current_frame][y][x];
                if (color_index >= 0 && color_index <= 9) {
                    tft.fillRect(8 + x * 7, 8 + y * 7, 7, 7, color_map[color_index]);
                }
            }
        }
        current_frame = (current_frame + 1) % images_in_history;
        lastAnimTime = millis();
    }
}

void process_image_part(int image_id, int part_number, const char* part_data) {
    Serial.print("Processing image part: ");
    Serial.print(image_id);
    Serial.print(", ");
    Serial.println(part_number);

    if (current_image_buffer.image_id != image_id) {
        memset(current_image_buffer.data, 0, sizeof(current_image_buffer.data));
        memset(current_image_buffer.received_parts, 0, sizeof(current_image_buffer.received_parts));
        current_image_buffer.image_id = image_id;
    }

    int start_index = (part_number - 1) * (1024 / 8);
    memcpy(current_image_buffer.data + start_index, part_data, strlen(part_data));
    current_image_buffer.received_parts[part_number - 1] = true;

    bool all_parts_received = true;
    for (int i = 0; i < 8; ++i) {
        if (!current_image_buffer.received_parts[i]) {
            all_parts_received = false;
            break;
        }
    }

    if (all_parts_received) {
        Serial.println("All parts received. Processing full image.");
        set_pixel_data(current_image_buffer.data);
        start_pixel_art();
        memset(current_image_buffer.data, 0, sizeof(current_image_buffer.data));
        memset(current_image_buffer.received_parts, 0, sizeof(current_image_buffer.received_parts));
        current_image_buffer.image_id = -1;
    }
}
