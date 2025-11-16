#include "WifiManager.h"

WifiManager::WifiManager(const char* apSSID, const char* apPassword)
    : apSSID(apSSID), apPassword(apPassword), server(80)
{
}

void WifiManager::Begin()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_MODE_APSTA);


    WiFi.softAP(apSSID, apPassword, 1, false, 1); // 1 = channel, false = hidden, 1 = max connections
    WiFi.softAPConfig(ap_local_ip, ap_gateway, ap_subnet);
    Serial.println("AP started");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    // Load credentials from flash memory
    preferences.begin("wificre", true);
    networkSSID = preferences.getString("ssid", "");
    networkPassword = preferences.getString("password", "");
    preferences.end();

    if (networkSSID != "" && networkPassword != "")
    {
        Serial.println("Connecting to stored Wi-Fi network...");
        connected = connectToWiFi(networkSSID, networkPassword);
        Serial.print("Connected: "); Serial.println(connected);
    }

    // Setup HTTP server routes 
    // Routes
    server.on("/wifi", HTTP_GET, [this](AsyncWebServerRequest *request){
        this->handleWifi(request);
    });

    server.on("/wifiset", HTTP_POST, [this](AsyncWebServerRequest *request){
        this->handleWifiSet(request);
    });
   
    server.begin();

    Serial.println("HTTP server started");
    setupMDNS();
}

bool WifiManager::CyclTask() 
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
            Serial.println("Wi-Fi connected.");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
        }
        else
        {
            Serial.println("Try to reconnect to Wi-Fi...");
            WiFi.begin(networkSSID.c_str(), networkPassword.c_str());
            wifiStatus = false;
        }
    }
    
    return wifiStatus;
  }


void WifiManager::handleWifi(AsyncWebServerRequest *request)
{
    String html = "<html><body><h1>Configure Wi-Fi</h1>";
    html += "<form action='/wifiset' method='POST'>";
    html += "SSID: <input type='text' name='ssid'><br>";
    html += "Password: <input type='password' name='password'><br>";
    html += "<input type='submit' value='Submit'></form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
}

void WifiManager::handleWifiSet(AsyncWebServerRequest *request)
{

    
    
    if (request->hasParam("ssid", true) && request->hasParam("password", true))
    {
        networkSSID = request->getParam("ssid", true)->value();
        networkPassword = request->getParam("password", true)->value();

        // Save credentials to flash memory
        preferences.begin("wificre", false);
        preferences.putString("ssid", networkSSID);
        preferences.putString("password", networkPassword);
        preferences.end();

        request->send(200, "text/html", "Credentials received. Connecting to network...");
        delay(2000);
        connectToWiFi(networkSSID, networkPassword);
    }
    else
    {
        request->send(400, "text/html", "Bad Request");
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
        Serial.print("Connected to Wi-Fi");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        return true;
    }
    else
    {
        Serial.println("Failed to connect to Wi-Fi");

        return false;
    }
}
