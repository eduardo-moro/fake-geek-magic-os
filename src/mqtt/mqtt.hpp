#ifndef MQTT_HPP
#define MQTT_HPP

#include "main.hpp"
#include "PubSubClient.h"
#include "WiFiClientSecure.h"

#define MQTT_BROKER   "a58bfb515e3d44e4bbec101bf408bb32.s1.eu.hivemq.cloud"
#define MQTT_PORT     8883
#define MQTT_TOPIC_COMMAND "esp/rgb/set"

extern WiFiClientSecure espClient;
extern PubSubClient client;

void setup_MQTT();
void attempt_MQTT_reconnect();
void on_MQTT_message(char* topic, byte* payload, unsigned int length);

#endif