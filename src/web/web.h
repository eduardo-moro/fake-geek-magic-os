#ifndef WEB_H
#define WEB_H

#include "LittleFS.h"
#include "Arduino.h"
#include "TFT_eSPI.h"
#include "DNSServer.h"
#include "../ui/animation/animation.h"
#include "EEPROM.h"

// Platform-specific includes
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#define WebServerType ESP8266WebServer
#elif defined(ESP32)
#include "WiFi.h"
#include "WebServer.h"
#define WebServerType WebServer
#endif

#define FS_NO_GLOBALS
#define MAX_NETWORKS 5
#define EEPROM_SIZE sizeof(NetworkConfig) * MAX_NETWORKS

struct NetworkConfig
{
    char ssid[32];
    char password[64];
    int last_connected; // Timestamp placeholder
};

struct WifiItem
{
    String ssid;
    String value;
    int signal_strength;
};

extern boolean ap_active;
extern boolean ap_connected;
extern unsigned long ap_active_time;
extern TFT_eSPI tft;
extern DNSServer dnsServer;
extern String route;
extern WebServerType server;

void start_ap();
void stop_ap();
void start_client(const char *ssid, const char *password);
void host_webpage();
void saveNetwork(const String &ssid, const String &password);
bool connectToBestNetwork();
void qr_code_timeout();
int getSignalStrengthLevel();
int getAvailableNetworksCount();
void connectToNetwork(const String &ssid);
std::vector<WifiItem> getAvailableNetworksMenuItems();

#endif