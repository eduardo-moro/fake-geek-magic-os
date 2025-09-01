#ifndef MQTT_HPP
#define MQTT_HPP

#include "PubSubClient.h"
#include "main.hpp"

#define MQTT_BROKER   "a58bfb515e3d44e4bbec101bf408bb32.s1.eu.hivemq.cloud"
#define MQTT_PORT     8883
#define MQTT_TOPIC_COMMAND "esp/rgb/set"

extern PubSubClient client;

void loop_MQTT();
void attempt_MQTT_reconnect();
void on_MQTT_message(char* topic, byte* payload, unsigned int length);

#endif