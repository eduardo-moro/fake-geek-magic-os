#ifndef WEB_H
#define WEB_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

void start_ap();
void stop_ap();
void start_client(const char *ssid, const char *password);

#endif