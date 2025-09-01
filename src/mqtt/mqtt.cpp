#include "mqtt.hpp"

const char* mqtt_username = "clock";
const char* mqtt_password = "Clock-01";

void setup_MQTT() {
    espClient.setInsecure();
    client.setServer(MQTT_BROKER, MQTT_PORT);
    
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    
    attempt_MQTT_reconnect();
}

void attempt_MQTT_reconnect() {
  Serial.println("Connecting to mqtt...");
  String client_id = "esp32-client-" + String(WiFi.macAddress());
  if (!client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
    Serial.print("MQTT reconnect failed, state=");
    Serial.println(client.state());
  }
}
