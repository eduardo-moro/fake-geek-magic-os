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
  } else {
    buddy.prompt_id[0] = 0;
    buddy.prompt_tool[0] = 0;
    buddy.prompt_hint[0] = 0;
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

static void process_json_line(const String& line) {
  if (line.length() == 0) return;

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, line);

  if (error) {
    Serial.printf("[buddy] JSON error: %s\n", error.c_str());
    return;
  }

  JsonObject obj = doc.as<JsonObject>();

  if (obj.containsKey("cmd")) {
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

static void draw_centered_text(const char* text, int y, int max_width_chars) {
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
      line_y += 12;  // Line spacing
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

  if (font_loaded) {
    // With smaller font, allow more chars per line
    draw_centered_text(buddy.prompt_hint, 105, 32);
  } else {
    // With default font, fewer chars per line
    draw_centered_text(buddy.prompt_hint, 160, 25);
  }

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
    Serial.println("[buddy] UTF-8 font loaded");
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
    } else if (!deny_button_already_handled && millis() - deny_button_pressed_time > 100) {
      buddy_app_send_deny();
      deny_button_already_handled = true;
    }
  } else {
    deny_button_pressed_time = 0;
    deny_button_already_handled = false;
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
