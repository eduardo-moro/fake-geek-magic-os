#ifndef MQTT_HPP
#define MQTT_HPP

#include "main.hpp"
#include "PubSubClient.h"
#include "WiFiClient.h"

#define MQTT_BROKER   "broker.emqx.io"
#define MQTT_PORT     1883
#define MQTT_TOPIC_COMMAND "ehpmcp/esp/rgb/set"
#define MQTT_TOPIC_PIXELS "ehpmcp/esp/pixel/set"

extern WiFiClient espClient;
extern PubSubClient client;

void setup_MQTT();
void attempt_MQTT_reconnect();
void on_MQTT_message(char* topic, byte* payload, unsigned int length);

#endif
