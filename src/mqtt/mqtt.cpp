#include "mqtt.hpp"

unsigned long lastMqttAttempt = 0;
const unsigned long mqttReconnectInterval = 500;

const char* mqtt_username = "esp01";
const char* mqtt_password = "Esp32_01";

void setup_MQTT() {
  espClient.setInsecure(); 
    client.setServer(MQTT_BROKER, MQTT_PORT);
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected!");
      client.publish("esp/status/clock", "online!");
    } else {
      Serial.print("MQTT connect failed, state=");
      Serial.println(client.state());
    }
}

void attempt_MQTT_reconnect() {
  Serial.println("Connecting to mqtt...");
  String client_id = "esp32-client-" + String(WiFi.macAddress());
  if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
    Serial.println("MQTT connected!");
    client.publish("esp/status/clock", "online!");
  } else {
    Serial.print("MQTT reconnect failed, state=");
    Serial.println(client.state());
  }
}
