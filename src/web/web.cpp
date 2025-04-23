#include "web.h"


extern boolean ap_active;
extern boolean ap_connected;
extern unsigned long ap_active_time;

void start_ap() 
{
    ap_active_time = millis();
    if (ap_active) return;

    WiFi.mode(WIFI_AP);
    while(!WiFi.softAP("moro's mini tv", "morinho0"))
    {
    Serial.print(".");
      delay(100);
    }
    
    ap_active = true;

    Serial.println("\nAP started");

    return;
}

void stop_ap() 
{
    if (!ap_active) return;

    WiFi.softAPdisconnect(true);
    ap_active = false;
    Serial.println("AP stopped");

    return;
}

void start_client(const char *ssid, const char *password) 
{
    if (ap_active) stop_ap();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("Connecting to WiFi...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    return;
}

// host index.html
