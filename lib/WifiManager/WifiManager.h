#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

class WifiManager
{
public:
    WifiManager(const char* apSSID, const char* apPassword);
    void Begin();
    bool CyclTask();

    AsyncWebServer& getServer() { return server; }

private:
    const char* apSSID;
    const char* apPassword;
    const IPAddress ap_local_ip = IPAddress(192, 168, 44, 1);
    const IPAddress ap_gateway = IPAddress(192, 168, 44, 254);
    const IPAddress ap_subnet = IPAddress(255, 255, 255, 0);
    Preferences preferences;
    String networkSSID = "";
    String networkPassword = "";


    void handleWifi(AsyncWebServerRequest *request);
    void handleWifiSet(AsyncWebServerRequest *request);
    bool connectToWiFi(const String& ssid, const String& password);
    void setupMDNS();    // configuration mDNS

    bool connected = false;
    const long checkWifiIntervalMs = 5000; // 5 seconds
    AsyncWebServer server;    // déplacé en membre privé
};

#endif
