#include "websocket.hpp"

void setupWebSocket() {
  IPAddress gateway = WiFi.gatewayIP();
  String gatewayStr = gateway.toString();

  Serial.print("Connecting WebSocket to: ");
  Serial.println(gatewayStr);

  webSocket.begin(gatewayStr, WS_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      currentNotification.hasUnread = false;
      break;

    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      break;

    case WStype_TEXT:
      Serial.printf("WebSocket Message: %s\n", payload);

      // Parse JSON notification
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        currentNotification.title = doc["title"].as<String>();
        currentNotification.message = doc["message"].as<String>();
        currentNotification.hasUnread = true;

        // Switch to notification display
        currentState = STATE_NOTIFICATION;
        scrollOffset = -INITIAL_OFFSET; // Start from right side
        notificationActive = true;
        lastScrollTime = millis();
      }
      break;
  }
}
