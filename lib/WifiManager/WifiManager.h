#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include <ESPmDNS.h>

class WifiManager
{
public:
    WifiManager(const char* apSSID, const char* apPassword);
    void begin();
    bool checkWiFiConnection();

private:
    const char* apSSID;
    const char* apPassword;
    const IPAddress ap_local_ip = IPAddress(192, 168, 1, 1);
    const IPAddress ap_gateway = IPAddress(192, 168, 1, 1);
    const IPAddress ap_subnet = IPAddress(255, 255, 255, 0);
    Preferences preferences;
    String networkSSID = "";
    String networkPassword = "";


    void handleRoot();
    void handleSet();
    void handleHello();  // nouveau endpoint pour "helloword"
    bool connectToWiFi(const String& ssid, const String& password);
    void setupMDNS();    // configuration mDNS

    bool connected = false;
    const long checkWifiIntervalMs = 5000; // 5 seconds
    WebServer server;    // déplacé en membre privé
};

#endif
