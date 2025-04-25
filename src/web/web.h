#ifndef WEB_H
#define WEB_H

#include <LittleFS.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <animation.h>
#include <EEPROM.h>

#define FS_NO_GLOBALS
#define MAX_NETWORKS 5
#define EEPROM_SIZE sizeof(NetworkConfig) * MAX_NETWORKS

struct NetworkConfig
{
    char ssid[32];
    char password[64];
    int last_connected; // Timestamp placeholder
};

extern boolean ap_active;
extern boolean ap_connected;
extern unsigned long ap_active_time;
extern TFT_eSPI tft;
extern DNSServer dnsServer;
extern String route;
extern ESP8266WebServer server;

void start_ap();
void stop_ap();
void start_client(const char *ssid, const char *password);
void host_webpage();
void saveNetwork(const String &ssid, const String &password);
bool connectToBestNetwork();
void qr_code_timeout();
int getSignalStrengthLevel();


#endif