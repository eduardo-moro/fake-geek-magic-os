#include "mqtt.hpp"
#include "pixels/pixels.hpp"
#include <ArduinoJson.h>

// scoll settings
#define SCROLL_DELAY 50
#define INITIAL_OFFSET 40

Notification currentNotification;
int notifyScrollOffset;
unsigned long lastScrollTime;

const byte XOR_KEY[] = {0x73, 0x75, 0x70, 0x65, 0x72, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74, 0x6b, 0x65, 0x79}; // "supersecretkey"

void decrypt_message(byte* payload, unsigned int length, char* decrypted_message) {
    int key_len = sizeof(XOR_KEY);
    for (unsigned int i = 0; i < length; i++) {
        decrypted_message[i] = payload[i] ^ XOR_KEY[i % key_len];
    }
    decrypted_message[length] = '\0'; // Null-terminate the decrypted message
}

void setup_MQTT() {
    // Serial.println("setup_MQTT called");
    // Serial.println("Setting up MQTT...");
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(on_MQTT_message);
    client.setBufferSize(MQTT_MAX_PACKET_SIZE);
    
    #if defined(ESP32)
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    #else
    String client_id = "esp8266-client-" + WiFi.macAddress();
    #endif
    
    attempt_MQTT_reconnect();
}

void attempt_MQTT_reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    // Serial.println("WiFi not connected, skipping MQTT reconnect.");
    return;
  }
  // Serial.println("Attempting MQTT connection...");
  #if defined(ESP32)
  String client_id = "esp32-client-" + String(WiFi.macAddress());
  #else
  String client_id = "esp8266-client-" + WiFi.macAddress();
  #endif

  if (!client.connect(client_id.c_str())) {
    // Serial.print("MQTT connection failed, rc=");
    // Serial.print(client.state());
    // Serial.println(" Retrying in 5 seconds...");
  } else {
    // Serial.println("MQTT connected!");
    client.publish("ehpmcp/esp/status/clock", "Clock is online!");
    if(client.subscribe(MQTT_TOPIC_PIXELS)) {
      // Serial.println("Subscribed to MQTT_TOPIC_PIXELS");
    } else {
      // Serial.println("Failed to subscribe to MQTT_TOPIC_PIXELS");
    }
    if(client.subscribe(MQTT_TOPIC_NOTIFICATIONS)) {
      // Serial.println("Subscribed to MQTT_TOPIC_NOTIFICATIONS");
    } else {
      // Serial.println("Failed to subscribe to MQTT_TOPIC_NOTIFICATIONS");
    }
  }
}

void on_MQTT_message(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Message arrived in topic: ");
  // Serial.println(topic);
  // Serial.print("Message length: ");
  // Serial.println(length);
  // Serial.print("MQTT buffer size: ");
  // Serial.println(MQTT_MAX_PACKET_SIZE);

  if (strcmp(topic, MQTT_TOPIC_PIXELS) == 0) {
    // Serial.println("Processing pixel data part from MQTT message...");
    payload[length] = '\0'; // Null-terminate the payload
    char* payload_str = (char*)payload;

    char* img_id_str = strtok(payload_str, ",");
    char* part_number_str = strtok(NULL, ",");
    char* image_data_str = strtok(NULL, ",");

    if (img_id_str != NULL && part_number_str != NULL && image_data_str != NULL) {
        int img_id = atoi(img_id_str);
        int part_number = atoi(part_number_str);
        process_image_part(img_id, part_number, image_data_str);
    } else {
        // Serial.println("Invalid MQTT message format for pixel data.");
    }
  } else if (strcmp(topic, MQTT_TOPIC_NOTIFICATIONS) == 0) {
    // Serial.println("Processing notification from MQTT message...");

    char decrypted_message[length + 1]; // +1 for null terminator
    decrypt_message(payload, length, decrypted_message);

    // Serial.printf("Decrypted message: %s\n", decrypted_message);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, decrypted_message);

    if (!error) {
        currentNotification.title = doc["title"].as<String>();
        currentNotification.message = doc["message"].as<String>();
        currentNotification.hasUnread = true;

        notifyScrollOffset = -INITIAL_OFFSET;
        lastScrollTime = millis();
      } else {
        // Serial.println("Failed to parse notification JSON");
      }
  }
}

void displayNotification() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  tft.setCursor(0, 0);

  String displayTitle = currentNotification.title;
  if (displayTitle.length() > 21) {
    displayTitle = displayTitle.substring(0, 18) + "...";
  }
  tft.println(displayTitle);

  tft.drawLine(0, 10, TFT_WIDTH, 10, TFT_WHITE);

  tft.setTextSize(2);

  int messageY = 12 + (TFT_HEIGHT - 12) / 2;

  int messageX = TFT_WIDTH - notifyScrollOffset;
  tft.setCursor(messageX, messageY);
  tft.print(currentNotification.message);

  unsigned long currentTime = millis();
  if (currentTime - lastScrollTime > SCROLL_DELAY) {
    notifyScrollOffset++;
    lastScrollTime = currentTime;

    if (notifyScrollOffset > TFT_WIDTH + INITIAL_OFFSET) {
      notifyScrollOffset = 0;
    }
  }
}
