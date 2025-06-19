#include "web.h"

#if defined(ESP8266)
int enc = ENC_TYPE_NONE;
#elif defined(ESP32)
int enc = WIFI_AUTH_OPEN;
#endif

void start_ap()
{
    ap_active_time = millis();
    if (ap_active)
        return;

    WiFi.mode(WIFI_AP);

    while (!WiFi.softAP("moro's mini tv", "morinho0"))
    {
        Serial.print(".");
        delay(100);
    }

    dnsServer.start(53, "*", WiFi.softAPIP());

    ap_active = true;
    Serial.println("\nAP started");
    host_webpage();

    qr_code_timeout();
}

void stop_ap()
{
    if (!ap_active)
        return;

    WiFi.softAPdisconnect(true);
    ap_active = false;
    Serial.println("AP stopped");

    return;
}

void qr_code_timeout()
{
    if ((millis() - ap_active_time > 10000 || ap_connected) && route != "menu")
    {
        route = "menu";
        setBrightnessPercent(40);
        initializeMenu();
        tft.drawString("ap on", 120, 14);

        if (ap_connected)
        {
            stop_ap();
        }
    }
}

void start_client(const char *ssid, const char *password)
{
    if (ap_active)
        stop_ap();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("Connecting to WiFi...");

    unsigned long start = millis();

    tft.fillScreen(TFT_BLACK);
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
        loop_wifi_icon();
        delay(100);
        Serial.print(".");
    }

    saveNetwork(ssid, password);

    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    initializeMenu();
    tft.setTextSize(1);
    tft.drawString(WiFi.localIP().toString(), 120, 14);
    return;
}

void serveImage(const char *path)
{
    fs::File file = LittleFS.open(path, "r");
    if (file)
    {
        server.sendHeader("Cache-Control", "public, max-age=604800");
        server.streamFile(file, "image/png");
        file.close();
    }
    else
    {
        server.send(404, "text/plain", "Image not found");
    }
};

void host_webpage()
{
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount LittleFS");
        return;
    }

    // Serve static files
    server.on("/", HTTP_GET, []()
              {
        fs::File file = LittleFS.open("/pages/wifi.html", "r");
        if (file) {
            server.streamFile(file, "text/html");
            file.close();
        } else {
            server.send(404, "text/plain", "File not found");
        } });

    const char *captivePaths[] = {
        "/generate_204",        // Android
        "/connecttest.txt",     // Windows
        "/hotspot-detect.html", // Apple
        "/ncsi.txt",            // Microsoft
        "/success.txt"          // Firefox OS
    };

    for (auto path : captivePaths)
    {
        server.on(path, []()
                  {
            server.sendHeader("Location", "http://192.168.4.1");
            server.send(302, "text/plain", ""); });
    }

    // Redirect all other requests
    server.onNotFound([]()
                      {
        server.sendHeader("Location", "http://192.168.4.1");
        server.send(302, "text/plain", "Redirecting to setup"); });

    // WiFi scan endpoint
    server.on("/scan", HTTP_GET, []()
              {

        String json = "{";
        json += "\"networks\":[";
        int n = WiFi.scanNetworks();
        for (int i = 0; i < n; ++i) {
            if (i) json += ",";
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
            json += "\"secure\":" + String(WiFi.encryptionType(i) != enc);
            json += "}";
        }
        json += "]}";
        server.send(200, "application/json", json); });

    // Connection endpoint
    server.on("/connect", HTTP_POST, []()
              {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        Serial.println("Connecting to SSID: " + ssid + ", Password: " + password);
        
        if (ssid.length() == 0) {
            server.send(400, "text/plain", "SSID required");
            return;
        }

        server.send(200, "text/plain", "Connecting...");
        start_client(ssid.c_str(), password.c_str()); });

    // Serve images
    server.on("/images/wifi_0.png", HTTP_GET, []()
              { serveImage("/pages/images/wifi_0.png"); });

    server.on("/images/wifi_1.png", HTTP_GET, []()
              { serveImage("/pages/images/wifi_1.png"); });

    server.on("/images/wifi_2.png", HTTP_GET, []()
              { serveImage("/pages/images/wifi_2.png"); });

    server.on("/images/wifi_3.png", HTTP_GET, []()
              { serveImage("/pages/images/wifi_3.png"); });

    server.on("/images/reload.png", HTTP_GET, []()
              { serveImage("/pages/images/reload.png"); });

    server.begin();
    Serial.println("HTTP server started");
}

void saveNetwork(const String &ssid, const String &password)
{
    EEPROM.begin(EEPROM_SIZE);

    NetworkConfig networks[MAX_NETWORKS];
    EEPROM.get(0, networks);

    // Check if already exists
    for (int i = 0; i < MAX_NETWORKS; i++)
    {
        if (String(networks[i].ssid) == ssid)
        {
            strncpy(networks[i].password, password.c_str(), sizeof(networks[i].password));
            EEPROM.put(0, networks);
            EEPROM.commit();
            EEPROM.end();
            return;
        }
    }

    // Find empty slot
    for (int i = 0; i < MAX_NETWORKS; i++)
    {
        if (networks[i].ssid[0] == '\0')
        {
            strncpy(networks[i].ssid, ssid.c_str(), sizeof(networks[i].ssid));
            strncpy(networks[i].password, password.c_str(), sizeof(networks[i].password));
            networks[i].last_connected = millis();
            EEPROM.put(0, networks);
            EEPROM.commit();
            EEPROM.end();
            return;
        }
    }

    // No space - replace oldest
    int oldest = 0;
    for (int i = 1; i < MAX_NETWORKS; i++)
    {
        if (networks[i].last_connected < networks[oldest].last_connected)
        {
            oldest = i;
        }
    }

    strncpy(networks[oldest].ssid, ssid.c_str(), sizeof(networks[oldest].ssid));
    strncpy(networks[oldest].password, password.c_str(), sizeof(networks[oldest].password));
    networks[oldest].last_connected = millis();

    EEPROM.put(0, networks);
    EEPROM.commit();
    EEPROM.end();
}

bool connectToBestNetwork()
{
    EEPROM.begin(EEPROM_SIZE);
    NetworkConfig networks[MAX_NETWORKS];
    EEPROM.get(0, networks);
    EEPROM.end();

    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();

    NetworkConfig bestNetwork;
    int bestRSSI = -1000;
    bool found = false;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < MAX_NETWORKS; j++)
        {
            if (String(networks[j].ssid) == WiFi.SSID(i) &&
                WiFi.RSSI(i) > bestRSSI)
            {
                bestRSSI = WiFi.RSSI(i);
                bestNetwork = networks[j];
                found = true;
            }
        }
    }

    if (found)
    {
        WiFi.begin(bestNetwork.ssid, bestNetwork.password);
        unsigned long start = millis();

        tft.fillScreen(TFT_BLACK);
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
        {
            loop_wifi_icon();
            delay(100);
            Serial.print(".");
        }

        return WiFi.status() == WL_CONNECTED;
    }
    return false;
}

int getSignalStrengthLevel()
{
    long rssi = WiFi.RSSI();

    if (rssi >= -60)
    {
        return 3; // Strong
    }
    else if (rssi >= -70)
    {
        return 2; // Moderate
    }
    else if (rssi >= -80)
    {
        return 1; // Weak
    }
    else
    {
        return 0; // Very weak
    }
}