#include "WifiManager.h"

WifiManager::WifiManager(const char* apSSID, const char* apPassword)
    : apSSID(apSSID), apPassword(apPassword), server(80)
{
}

void WifiManager::begin()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    // Load credentials from flash memory
    preferences.begin("wificre", true);
    networkSSID = preferences.getString("ssid", "");
    networkPassword = preferences.getString("password", "");
    preferences.end();

    if (networkSSID != "" && networkPassword != "")
    {
        Serial.println("Connecting to stored Wi-Fi network...");
        connected = connectToWiFi(networkSSID, networkPassword);
    }

    if(!connected)
    {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(apSSID, apPassword, 1, false, 1); // 1 = channel, false = hidden, 1 = max connections
        WiFi.softAPConfig(ap_local_ip, ap_gateway, ap_subnet);
        Serial.println("AP started");
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());

        server.on("/", std::bind(&WifiManager::handleRoot, this));
        server.on("/set", HTTP_POST, std::bind(&WifiManager::handleSet, this));
        server.on("/hello", HTTP_GET, std::bind(&WifiManager::handleHello, this));
        server.begin();
        Serial.println("HTTP server started");
        
        setupMDNS();
    }

}

bool WifiManager::checkWiFiConnection() 
{
    static bool wifiStatus = false;
    unsigned long currentMillis = millis();
    static unsigned long previousMillis = 0;

    
    // check if the connection is still alive
    // if not try to reconnect
    if (currentMillis - previousMillis >= checkWifiIntervalMs)
    {
        previousMillis = currentMillis;
        if(WiFi.status() == WL_CONNECTED)
        {
            wifiStatus = true;
        }
        else
        {
            Serial.println("Try to reconnect to Wi-Fi...");
            WiFi.begin(networkSSID.c_str(), networkPassword.c_str());
            wifiStatus = false;
        }
    }
    
    if(connected == false)
    {
        // if never connected, manager the HTTP server in AP mode (for the Wi-Fi configuration)
        server.handleClient();
    }

    return wifiStatus;
  }

void WifiManager::handleRoot()
{
    String html = "<html><body><h1>Configure Wi-Fi</h1>";
    html += "<form action='/set' method='POST'>";
    html += "SSID: <input type='text' name='ssid'><br>";
    html += "Password: <input type='password' name='password'><br>";
    html += "<input type='submit' value='Submit'></form>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void WifiManager::handleSet()
{
    if (server.hasArg("ssid") && server.hasArg("password"))
    {
        networkSSID = server.arg("ssid");
        networkPassword = server.arg("password");

        // Save credentials to flash memory
        preferences.begin("wificre", false);
        preferences.putString("ssid", networkSSID);
        preferences.putString("password", networkPassword);
        preferences.end();

        server.send(200, "text/html", "Credentials received. Connecting to network...");
        delay(2000);
        connectToWiFi(networkSSID, networkPassword);
    }
    else
    {
        server.send(400, "text/html", "Bad Request");
    }
}

void WifiManager::setupMDNS() {
    if(!MDNS.begin("cde")) {
        Serial.println("Error setting up MDNS responder!");
        return;
    }
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started at http://cde.local");
}

void WifiManager::handleHello() {
    server.send(200, "text/plain", "helloword");
}

bool WifiManager::connectToWiFi(const String& ssid, const String& password)
{
    WiFi.begin(ssid.c_str(), password.c_str());
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20)
    {
        delay(500);
        Serial.print(".");
        attempt++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected to Wi-Fi");
        server.send(200, "text/html", "Connected to Wi-Fi");

        return true;
    }
    else
    {
        Serial.println("Failed to connect to Wi-Fi");
        server.send(200, "text/html", "Failed to connect to Wi-Fi");

        return false;
    }
}
