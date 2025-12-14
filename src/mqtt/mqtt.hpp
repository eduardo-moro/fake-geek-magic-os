#ifndef MQTT_HPP
#define MQTT_HPP
#define MQTT_MAX_PACKET_SIZE 512

#include "main.hpp"
#include "PubSubClient.h"
#include "WiFiClient.h"
#include "TFT_eSPI.h"

#define MQTT_BROKER   "broker.emqx.io"
#define MQTT_PORT     1883
#define MQTT_TOPIC_COMMAND "ehpmcp/esp/rgb/set"
#define MQTT_TOPIC_PIXELS "ehpmcp/esp/pixel/set"
#define MQTT_TOPIC_NOTIFICATIONS "ehpmcp/esp/notification"

struct Notification {
  String title;
  String message;
  bool hasUnread;
};

extern WiFiClient espClient;
extern PubSubClient client;
extern Notification currentNotification;
extern int notifyScrollOffset;
extern unsigned long lastScrollTime;

void setup_MQTT();
void attempt_MQTT_reconnect();
void on_MQTT_message(char* topic, byte* payload, unsigned int length);
void displayNotification();

#endif
