#ifndef MQTT_HPP
#define MQTT_HPP
#define MQTT_MAX_PACKET_SIZE 256

#include "main.hpp"
#include "PubSubClient.h"
#include "WiFiClient.h"

// IMPORTANT: By default, PubSubClient has a buffer size of 256 bytes.
// If you are sending messages larger than this, you need to increase the buffer size.
// You can do this by adding the following line before including PubSubClient.h:

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
