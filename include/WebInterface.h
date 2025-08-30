#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "Config.h"

class WebInterface {
private:
    WebServer* server;
    bool wifiConnected;
    String localIP;
    
public:
    WebInterface();
    ~WebInterface();
    
    bool begin();
    void update(SystemState state, float currentTemp, float targetTemp, 
               unsigned long remainingTime, float power);
    
    bool isConnected() { return wifiConnected; }
    String getIP() { return localIP; }
    
private:
    bool connectWiFi();
    void setupRoutes();
    void handleRoot();
    void handleStatus();
    void handleControl();
    void handleSettings();
    void handleNotFound();
    
    String generateHTML();
    String generateJSON(SystemState state, float currentTemp, float targetTemp, 
                       unsigned long remainingTime, float power);
};

#endif // WEB_INTERFACE_H