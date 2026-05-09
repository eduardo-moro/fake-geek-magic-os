#include "buddy_app.hpp"
#include "ble_bridge.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "../display/display.hpp"

extern TFT_eSPI tft;

static bool font_loaded = false;

#ifdef ESP32
#define DENY_BUTTON_PIN 0
#endif

enum BuddyState {
  BUDDY_DISCONNECTED,
  BUDDY_IDLE,
  BUDDY_BUSY,
  BUDDY_ATTENTION
};

struct TerminalLine {
  char text[120];
  uint16_t color;
};

static const int TERM_MAX_LINES = 16;  // 240px screen: (240 - 12 padding) / 14px per line = ~16 lines
static TerminalLine term_lines[TERM_MAX_LINES];
static int term_count = 0;
static int term_head = 0;  // Circular buffer head
static bool terminal_mode = false;
static unsigned long button_b_press_time = 0;
static bool terminal_dirty = true;  // Redraw when true
static uint16_t term_next_color = TFT_WHITE;  // Color for next println

struct BuddyContext {
  BuddyState state;
  int total_sessions;
  int running_sessions;
  int waiting_sessions;
  char msg[128];
  char entry[128];
  uint32_t tokens;
  uint32_t tokens_today;
  char prompt_id[64];
  char prompt_tool[32];
  char prompt_hint[96];
  bool prompt_pending;
  unsigned long last_heartbeat;
};

static BuddyContext buddy;

static void init_buddy_context() {
  buddy.state = BUDDY_DISCONNECTED;
  buddy.total_sessions = 0;
  buddy.running_sessions = 0;
  buddy.waiting_sessions = 0;
  buddy.msg[0] = '\0';
  buddy.entry[0] = '\0';
  buddy.tokens = 0;
  buddy.tokens_today = 0;
  buddy.prompt_id[0] = '\0';
  buddy.prompt_tool[0] = '\0';
  buddy.prompt_hint[0] = '\0';
  buddy.prompt_pending = false;
  buddy.last_heartbeat = 0;
}

static String line_buffer;
static unsigned long deny_button_pressed_time = 0;
static bool deny_button_already_handled = false;
static char last_prompt_id[64] = "";  // Track to avoid duplicate terminal lines

static void draw_terminal();  // Forward declaration

static void term_push(const char* text) {
  int slot;
  if (term_count < TERM_MAX_LINES) {
    slot = (term_head + term_count) % TERM_MAX_LINES;
    term_count++;
  } else {
    slot = term_head;
    term_head = (term_head + 1) % TERM_MAX_LINES;
  }

  strncpy(term_lines[slot].text, text, sizeof(term_lines[slot].text) - 1);
  term_lines[slot].text[sizeof(term_lines[slot].text) - 1] = '\0';
  term_lines[slot].color = term_next_color;
  term_next_color = TFT_WHITE;  // Reset color after each line

  terminal_dirty = true;
  if (terminal_mode) {
    draw_terminal();
  }
}

static void term_setColor(uint16_t color) {
  term_next_color = color;
}

static inline int term_bufIndex(int row) {
  return (term_head + row) % TERM_MAX_LINES;
}

static void process_heartbeat(const JsonObject& obj) {
  buddy.total_sessions = obj["total"] | 0;
  buddy.running_sessions = obj["running"] | 0;
  buddy.waiting_sessions = obj["waiting"] | 0;

  const char* msg = obj["msg"];
  if (msg) {
    strncpy(buddy.msg, msg, sizeof(buddy.msg) - 1);
    buddy.msg[sizeof(buddy.msg) - 1] = '\0';
  }

  buddy.tokens = obj["tokens"] | 0;
  buddy.tokens_today = obj["tokens_today"] | 0;

  JsonObject pr = obj["prompt"];
  buddy.prompt_pending = !pr.isNull();
  if (buddy.prompt_pending) {
    const char* pid = pr["id"];
    const char* pt = pr["tool"];
    const char* ph = pr["hint"];
    strncpy(buddy.prompt_id, pid ? pid : "", sizeof(buddy.prompt_id) - 1);
    buddy.prompt_id[sizeof(buddy.prompt_id) - 1] = '\0';
    strncpy(buddy.prompt_tool, pt ? pt : "", sizeof(buddy.prompt_tool) - 1);
    buddy.prompt_tool[sizeof(buddy.prompt_tool) - 1] = '\0';
    strncpy(buddy.prompt_hint, ph ? ph : "", sizeof(buddy.prompt_hint) - 1);
    buddy.prompt_hint[sizeof(buddy.prompt_hint) - 1] = '\0';
    Serial.printf("[buddy] prompt: %s - %s\n", buddy.prompt_tool, buddy.prompt_hint);

    // Add to terminal if new prompt
    if (strcmp(buddy.prompt_id, last_prompt_id) != 0) {
      String perm_line = ":: ";
      perm_line += buddy.prompt_tool;
      perm_line += " - ";
      perm_line += buddy.prompt_hint;
      term_setColor(TFT_YELLOW);
      term_push(perm_line.c_str());
      term_setColor(0x8410);  // Dark gray
      term_push("[S]=approve, [N]=deny");
      strncpy(last_prompt_id, buddy.prompt_id, sizeof(last_prompt_id) - 1);
      last_prompt_id[sizeof(last_prompt_id) - 1] = '\0';
    }
  } else {
    buddy.prompt_id[0] = 0;
    buddy.prompt_tool[0] = 0;
    buddy.prompt_hint[0] = 0;
    last_prompt_id[0] = 0;
  }

  JsonArray entries = obj["entries"];
  if (entries.size() > 0) {
    const char* first = entries[0];
    if (first) {
      strncpy(buddy.entry, first, sizeof(buddy.entry) - 1);
      buddy.entry[sizeof(buddy.entry) - 1] = '\0';
    }
  }

  buddy.last_heartbeat = millis();
}

static void process_command(const JsonObject& obj) {
  const char* cmd = obj["cmd"];
  if (!cmd) return;

  if (strcmp(cmd, "status") == 0) {
    StaticJsonDocument<512> response;
    response["ack"] = "status";
    response["ok"] = true;
    JsonObject data = response.createNestedObject("data");
    data["name"] = "Claude-Buddy";
    data["owner"] = "";
    data["sec"] = bleSecure();

    JsonObject bat = data.createNestedObject("bat");
    bat["pct"] = 85;
    bat["mV"] = 4100;
    bat["mA"] = 50;
    bat["usb"] = false;

    JsonObject sys = data.createNestedObject("sys");
    sys["up"] = millis() / 1000;
    sys["heap"] = ESP.getFreeHeap();
    sys["fsFree"] = 0;
    sys["fsTotal"] = 0;

    JsonObject stats = data.createNestedObject("stats");
    stats["appr"] = 0;
    stats["deny"] = 0;
    stats["vel"] = 0;
    stats["nap"] = 0;
    stats["lvl"] = 0;

    String json_str;
    serializeJson(response, json_str);
    json_str += "\n";
    bleWrite((const uint8_t*)json_str.c_str(), json_str.length());
  } else if (strcmp(cmd, "owner") == 0 || strcmp(cmd, "name") == 0) {
    StaticJsonDocument<64> response;
    response["ack"] = cmd;
    response["ok"] = true;
    String json_str;
    serializeJson(response, json_str);
    json_str += "\n";
    bleWrite((const uint8_t*)json_str.c_str(), json_str.length());
  } else if (strcmp(cmd, "unpair") == 0) {
    bleClearBonds();
    StaticJsonDocument<64> response;
    response["ack"] = "unpair";
    response["ok"] = true;
    String json_str;
    serializeJson(response, json_str);
    json_str += "\n";
    size_t written = bleWrite((const uint8_t*)json_str.c_str(), json_str.length());
    Serial.printf("[buddy] unpair ack sent (%d bytes)\n", written);
  }
}

static void process_evt(const JsonObject& obj) {
  const char* role = obj["role"];
  if (!role) return;

  if (strcmp(role, "user") == 0) {
    const char* content = obj["content"];
    if (content) {
      String display_text = "> ";
      display_text += content;
      term_setColor(TFT_CYAN);
      term_push(display_text.c_str());
      Serial.printf("[buddy] terminal: user: %s\n", content);
    }
  } else if (strcmp(role, "assistant") == 0) {
    JsonArray content = obj["content"];
    if (!content.isNull()) {
      for (JsonVariant item : content) {
        const char* type_str = item["type"];
        if (type_str && strcmp(type_str, "text") == 0) {
          const char* text = item["text"];
          if (text) {
            // Split long text into multiple lines if needed
            String full_text = text;
            int pos = 0;
            const int max_len = 36;  // chars per line at 10pt font

            while (pos < (int)full_text.length()) {
              int end = pos + max_len;
              if (end >= (int)full_text.length()) {
                String line = full_text.substring(pos);
                term_setColor(TFT_WHITE);
                term_push(line.c_str());
                break;
              } else {
                // Find last space before end
                int space_pos = end;
                while (space_pos > pos && full_text[space_pos] != ' ') {
                  space_pos--;
                }
                if (space_pos == pos) space_pos = end;

                String line = full_text.substring(pos, space_pos);
                term_setColor(TFT_WHITE);
                term_push(line.c_str());
                pos = space_pos + 1;
              }
            }
          }
        }
      }
    }
  }
}

static void process_json_line(const String& line) {
  if (line.length() == 0) return;

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, line);

  if (error) {
    Serial.printf("[buddy] JSON error: %s\n", error.c_str());
    return;
  }

  JsonObject obj = doc.as<JsonObject>();

  if (obj["evt"].as<const char*>() != nullptr) {
    process_evt(obj);
  } else if (obj.containsKey("cmd")) {
    process_command(obj);
  } else {
    process_heartbeat(obj);
  }
}

static void read_ble_lines() {
  while (bleAvailable() > 0) {
    int b = bleRead();
    if (b < 0) break;

    if (b == '\n') {
      if (line_buffer.length() > 0) {
        Serial.printf("[buddy] rx: %s\n", line_buffer.c_str());
        process_json_line(line_buffer);
      }
      line_buffer = "";
    } else if (b >= 32 && b < 127) {
      line_buffer += (char)b;
    } else if (b >= 0x80) {
      // UTF-8 byte (part of multibyte sequence)
      line_buffer += (char)b;
    }
  }
}

static void draw_disconnected() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4);
  tft.drawString("CLAUDE", 120, 100);
  tft.setTextSize(2);
  tft.drawString("Disconnected", 120, 150);
  tft.setTextSize(1);
  tft.drawString("Select from menu to connect", 120, 200);
}

static void draw_pairing() {
  uint32_t pk = blePasskey();
  if (pk == 0) return;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.drawString("PAIRING", 120, 60);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Enter on Desktop:", 120, 120);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(5);
  char passkey_str[8];
  snprintf(passkey_str, sizeof(passkey_str), "%06lu", (unsigned long)pk);
  tft.drawString(passkey_str, 120, 160);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Hold button to cancel", 120, 210);
}

static void draw_idle() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.drawString("CLAUDE", 120, 60);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  char buf[128];
  snprintf(buf, sizeof(buf), "Sessions: %d", buddy.total_sessions);
  tft.drawString(buf, 20, 120);

  snprintf(buf, sizeof(buf), "Tokens: %lu", buddy.tokens_today);
  tft.drawString(buf, 20, 150);

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.drawString("Idle - waiting for action", 120, 200);
}

static void draw_busy() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.drawString("WORKING", 120, 60);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  char buf[128];
  snprintf(buf, sizeof(buf), "Running: %d", buddy.running_sessions);
  tft.drawString(buf, 20, 120);

  tft.setTextSize(1);
  tft.drawString(buddy.entry, 20, 160);
}

static void draw_terminal() {
  if (!terminal_dirty) return;

  // Load 10pt font for terminal
  tft.unloadFont();
  tft.loadFont("UTF8-Latin1-10", LittleFS);

  tft.setTextDatum(TL_DATUM);
  tft.setTextWrap(false);  // Prevent TFT from auto-wrapping (would advance Y unexpectedly)
  tft.setTextSize(1);

  // Measure actual font height so rows never overlap
  const int line_height = tft.fontHeight() + 2;  // +2px leading
  const int padding_top = 6;
  const int max_visible_lines = (240 - padding_top - 6) / line_height;

  // Draw all lines in circular buffer using drawString (TL_DATUM: y = top of glyph, not baseline)
  for (int row = 0; row < term_count; row++) {
    int idx = term_bufIndex(row);
    const TerminalLine& tline = term_lines[idx];
    int y = padding_top + row * line_height;

    // Fill row background first to erase any previous content at this y
    tft.fillRect(0, y, 240, line_height, TFT_BLACK);
    tft.setTextColor(tline.color, TFT_BLACK);
    tft.drawString(tline.text, 0, y);
  }

  // Erase rows below content
  for (int row = term_count; row < max_visible_lines; row++) {
    int y = padding_top + row * line_height;
    tft.fillRect(0, y, 240, line_height, TFT_BLACK);
  }

  // Draw status indicator if Claude is processing
  if (buddy.running_sessions > 0 && term_count < max_visible_lines) {
    int y = padding_top + term_count * line_height;
    tft.fillRect(0, y, 240, line_height, TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("[Claude thinking...]", 0, y);
  }

  terminal_dirty = false;

  tft.setTextWrap(true);  // Restore wrap for other parts of the UI

  // Reload 16pt font
  tft.unloadFont();
  tft.loadFont("UTF8-Latin1-16", LittleFS);
}

static void draw_centered_text(const char* text, int y, int max_width_chars, int line_spacing) {
  // Draw text with word wrap, centered
  String full_text = text;
  int pos = 0;
  int line_y = y;

  while (pos < (int)full_text.length()) {
    int line_end = pos + max_width_chars;

    if (line_end >= (int)full_text.length()) {
      // Last line
      String line = full_text.substring(pos);
      tft.drawString(line.c_str(), 120, line_y);
      break;
    } else {
      // Find last space before line_end to break at word boundary
      int space_pos = line_end;
      while (space_pos > pos && full_text[space_pos] != ' ') {
        space_pos--;
      }

      if (space_pos == pos) {
        // No space found, break at max_width
        space_pos = line_end;
      }

      String line = full_text.substring(pos, space_pos);
      tft.drawString(line.c_str(), 120, line_y);
      line_y += line_spacing;
      pos = space_pos + 1;  // Skip the space
    }
  }
}

static void draw_attention() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  if (font_loaded) {
    tft.setTextSize(2);
    tft.drawString("APPROVE?", 120, 40);
  } else {
    tft.setTextSize(4);
    tft.drawString("APPROVE?", 120, 50);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  if (font_loaded) {
    tft.setTextSize(1);
    tft.drawString(buddy.prompt_tool, 120, 70);
  } else {
    tft.setTextSize(2);
    tft.drawString(buddy.prompt_tool, 120, 120);
  }

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Use 10pt font for hint text to fit more
  tft.unloadFont();
  tft.loadFont("UTF8-Latin1-10", LittleFS);
  draw_centered_text(buddy.prompt_hint, 110, 36, 10);
  // Reload 16pt for the rest
  tft.unloadFont();
  tft.loadFont("UTF8-Latin1-16", LittleFS);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  if (font_loaded) {
    tft.setTextSize(1);
    tft.drawString("TAP=OK", 75, 190);
  } else {
    tft.drawString("TAP=OK", 80, 210);
  }

  tft.setTextColor(TFT_RED, TFT_BLACK);
  if (font_loaded) {
    tft.setTextSize(1);
    tft.drawString("BTN=NO", 165, 190);
  } else {
    tft.drawString("BTN=NO", 160, 210);
  }
}

static void update_display() {
  uint32_t pk = blePasskey();
  if (pk > 0) {
    draw_pairing();
    return;
  }

  if (terminal_mode) {
    draw_terminal();
    return;
  }

  switch (buddy.state) {
    case BUDDY_DISCONNECTED:
      draw_disconnected();
      break;
    case BUDDY_IDLE:
      draw_idle();
      break;
    case BUDDY_BUSY:
      draw_busy();
      break;
    case BUDDY_ATTENTION:
      draw_attention();
      break;
  }
}

static void send_permission(const char* decision) {
  if (buddy.prompt_id[0] == '\0') return;

  StaticJsonDocument<256> doc;
  doc["cmd"] = "permission";
  doc["id"] = buddy.prompt_id;
  doc["decision"] = decision;

  String json_str;
  serializeJson(doc, json_str);
  json_str += "\n";

  bleWrite((const uint8_t*)json_str.c_str(), json_str.length());
  buddy.prompt_pending = false;
  buddy.prompt_id[0] = '\0';
}

void buddy_app_send_approve() {
  // Button A (cap-touch pin 32) → approve permission prompt
  if (buddy.prompt_pending && buddy.prompt_id[0] != '\0') {
    Serial.println("[buddy] button A: sending approve");
    send_permission("once");
  }
}

void buddy_app_send_deny() {
  // Button B (physical push pin 0) → deny permission prompt
  if (buddy.prompt_pending && buddy.prompt_id[0] != '\0') {
    Serial.println("[buddy] button B: sending deny");
    send_permission("deny");
  }
}

void buddy_app_start() {
  setBrightnessPercent(40);
  init_buddy_context();
  line_buffer = "";

  // Load UTF-8 capable font from LittleFS (with Latin-1 accents: ç, á, é, í, ó, ú, ã, õ)
  if (LittleFS.begin()) {
    tft.loadFont("UTF8-Latin1-16", LittleFS);
    font_loaded = true;
    Serial.println("[buddy] UTF-8 16pt font loaded");
  } else {
    Serial.println("[buddy] LittleFS failed");
  }

#ifdef ESP32
  pinMode(DENY_BUTTON_PIN, INPUT_PULLUP);
  Serial.println("[buddy] waiting for BLE connection...");
#endif

  update_display();
}

void buddy_app_loop() {
  read_ble_lines();  // Process incoming heartbeats and update buddy state

  static uint32_t last_passkey = 0;
  uint32_t current_passkey = blePasskey();
  if (current_passkey != last_passkey) {
    last_passkey = current_passkey;
    update_display();
  }

  static bool was_connected = false;
  bool is_connected = bleConnected();

  BuddyState prev_state = buddy.state;

  // Update state based on current conditions
  if (!is_connected) {
    buddy.state = BUDDY_DISCONNECTED;
    buddy.total_sessions = 0;
    buddy.running_sessions = 0;
    buddy.waiting_sessions = 0;
    buddy.prompt_pending = false;
  } else if (buddy.prompt_pending) {
    buddy.state = BUDDY_ATTENTION;
  } else if (buddy.running_sessions > 0) {
    buddy.state = BUDDY_BUSY;
  } else {
    buddy.state = BUDDY_IDLE;
  }

  if (buddy.state != prev_state) {
    Serial.printf("[buddy] state: %d → %d\n", prev_state, buddy.state);
    update_display();
  }

  was_connected = is_connected;

#ifdef ESP32
  if (digitalRead(DENY_BUTTON_PIN) == LOW) {
    if (deny_button_pressed_time == 0) {
      deny_button_pressed_time = millis();
      deny_button_already_handled = false;
    }
  } else {
    // Button released
    if (deny_button_pressed_time > 0 && !deny_button_already_handled) {
      unsigned long press_duration = millis() - deny_button_pressed_time;
      if (press_duration >= 500) {
        // Long press: toggle terminal mode
        terminal_mode = !terminal_mode;
        terminal_dirty = true;  // Force redraw on toggle
        Serial.printf("[buddy] button B: toggle terminal mode = %d\n", terminal_mode);
        update_display();
      } else {
        // Short press: send deny if prompt pending
        buddy_app_send_deny();
      }
      deny_button_already_handled = true;
    }
    deny_button_pressed_time = 0;
  }
#endif

  static unsigned long last_status_request = 0;
  unsigned long now = millis();
  if (is_connected && (now - last_status_request > 3000)) {
    StaticJsonDocument<64> doc;
    doc["cmd"] = "status";
    String json_str;
    serializeJson(doc, json_str);
    json_str += "\n";
    if (bleWrite((const uint8_t*)json_str.c_str(), json_str.length()) > 0) {
      last_status_request = now;
    }
  }

  if (buddy.last_heartbeat > 0 && now - buddy.last_heartbeat > 30000) {
    Serial.println("[buddy] heartbeat timeout");
    buddy.state = BUDDY_DISCONNECTED;
    buddy.total_sessions = 0;
    buddy.running_sessions = 0;
    buddy.waiting_sessions = 0;
    buddy.prompt_pending = false;
    update_display();
  }
}

void buddy_app_quit() {
  line_buffer = "";
  if (font_loaded) {
    tft.unloadFont();
    font_loaded = false;
  }
}
