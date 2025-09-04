#include "mqtt.hpp"

void setup_MQTT() {
    Serial.println("setup_MQTT called");
    Serial.println("Setting up MQTT...");
    client.setServer(MQTT_BROKER, MQTT_PORT);
    
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
  }
}