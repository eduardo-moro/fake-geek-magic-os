#include "mqtt.hpp"

unsigned long lastMqttAttempt = 0;
const unsigned long mqttReconnectInterval = 500;

const char* mqtt_username = "esp01";
const char* mqtt_password = "Esp32_01";

void loop_MQTT() {
    static unsigned long lastMqttAttempt = 0;
    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
        if (millis() - lastMqttAttempt > mqttReconnectInterval) {
        lastMqttAttempt = millis();
        attempt_MQTT_reconnect();
        }
    }
    client.loop();
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
