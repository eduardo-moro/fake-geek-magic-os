#include "mqtt.hpp"
#include "pixels/pixels.hpp"

void setup_MQTT() {
    Serial.println("setup_MQTT called");
    Serial.println("Setting up MQTT...");
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(on_MQTT_message);
    
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    
    attempt_MQTT_reconnect();
}

void attempt_MQTT_reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping MQTT reconnect.");
    return;
  }
  Serial.println("Attempting MQTT connection...");
  String client_id = "esp32-client-" + String(WiFi.macAddress());
  if (!client.connect(client_id.c_str())) {
    Serial.print("MQTT connection failed, rc=");
    Serial.print(client.state());
    Serial.println(" Retrying in 5 seconds...");
  } else {
    Serial.println("MQTT connected!");
    client.publish("ehpmcp/esp/status/clock", "Clock is online!");
    if(client.subscribe(MQTT_TOPIC_PIXELS)) {
      Serial.println("Subscribed to MQTT_TOPIC_PIXELS");
    } else {
      Serial.println("Failed to subscribe to MQTT_TOPIC_PIXELS");
    }
  }
}

void on_MQTT_message(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message length: ");
  Serial.println(length);
  Serial.print("MQTT buffer size: ");
  Serial.println(MQTT_MAX_PACKET_SIZE);

  if (strcmp(topic, MQTT_TOPIC_PIXELS) == 0) {
    Serial.println("Processing pixel data part from MQTT message...");
    payload[length] = '\0'; // Null-terminate the payload
    char* payload_str = (char*)payload;

    // Parse the message: "img_id,part_number,image_data"
    char* img_id_str = strtok(payload_str, ",");
    char* part_number_str = strtok(NULL, ",");
    char* image_data_str = strtok(NULL, ",");

    if (img_id_str != NULL && part_number_str != NULL && image_data_str != NULL) {
        int img_id = atoi(img_id_str);
        int part_number = atoi(part_number_str);
        process_image_part(img_id, part_number, image_data_str);
    } else {
        Serial.println("Invalid MQTT message format for pixel data.");
    }
  }
}
