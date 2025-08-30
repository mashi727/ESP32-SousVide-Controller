#include "../include/WebInterface.h"

WebInterface::WebInterface() {
    server = nullptr;
    wifiConnected = false;
    localIP = "";
}

WebInterface::~WebInterface() {
    if (server != nullptr) {
        delete server;
    }
}

bool WebInterface::begin() {
    if (!connectWiFi()) {
        return false;
    }
    
    server = new WebServer(WEB_SERVER_PORT);
    setupRoutes();
    server->begin();
    
    DEBUG_PRINTLN(F("Web server started"));
    return true;
}

void WebInterface::update(SystemState state, float currentTemp, float targetTemp, 
                         unsigned long remainingTime, float power) {
    if (server != nullptr) {
        server->handleClient();
    }
}

bool WebInterface::connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        DEBUG_PRINT(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        localIP = WiFi.localIP().toString();
        DEBUG_PRINT(F("\nWiFi connected. IP: "));
        DEBUG_PRINTLN(localIP);
        return true;
    }
    
    return false;
}

void WebInterface::setupRoutes() {
    server->on("/", [this]() { handleRoot(); });
    server->on("/status", [this]() { handleStatus(); });
    server->on("/control", [this]() { handleControl(); });
    server->on("/settings", [this]() { handleSettings(); });
    server->onNotFound([this]() { handleNotFound(); });
}

void WebInterface::handleRoot() {
    server->send(200, "text/html", generateHTML());
}

void WebInterface::handleStatus() {
    // This would need access to actual system state
    String json = "{\"status\":\"ok\"}";
    server->send(200, "application/json", json);
}

void WebInterface::handleControl() {
    if (server->hasArg("action")) {
        String action = server->arg("action");
        // Process control commands
        server->send(200, "text/plain", "OK");
    } else {
        server->send(400, "text/plain", "Bad Request");
    }
}

void WebInterface::handleSettings() {
    // Handle settings updates
    server->send(200, "text/plain", "Settings updated");
}

void WebInterface::handleNotFound() {
    server->send(404, "text/plain", "Not Found");
}

String WebInterface::generateHTML() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Sous Vide Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; }
        h1 { color: #333; text-align: center; }
        .status { margin: 20px 0; padding: 15px; background: #e8f4f8; border-radius: 5px; }
        .control { margin: 20px 0; }
        button { padding: 10px 20px; margin: 5px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
        button:hover { background: #45a049; }
        .temp { font-size: 24px; font-weight: bold; color: #2196F3; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Sous Vide Controller</h1>
        <div class="status">
            <p>Current Temperature: <span class="temp" id="currentTemp">--</span>°C</p>
            <p>Target Temperature: <span class="temp" id="targetTemp">--</span>°C</p>
            <p>Time Remaining: <span id="timeRemaining">--:--</span></p>
            <p>Power: <span id="power">--%</span></p>
        </div>
        <div class="control">
            <button onclick="startCooking()">Start</button>
            <button onclick="stopCooking()">Stop</button>
            <button onclick="pauseCooking()">Pause</button>
        </div>
    </div>
    <script>
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('currentTemp').textContent = data.currentTemp.toFixed(1);
                    document.getElementById('targetTemp').textContent = data.targetTemp.toFixed(1);
                    document.getElementById('timeRemaining').textContent = formatTime(data.remainingTime);
                    document.getElementById('power').textContent = data.power.toFixed(0) + '%';
                });
        }
        
        function formatTime(seconds) {
            const hours = Math.floor(seconds / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            const secs = seconds % 60;
            return hours > 0 ? 
                `${hours}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}` :
                `${minutes}:${secs.toString().padStart(2, '0')}`;
        }
        
        function startCooking() { fetch('/control?action=start'); }
        function stopCooking() { fetch('/control?action=stop'); }
        function pauseCooking() { fetch('/control?action=pause'); }
        
        setInterval(updateStatus, 1000);
        updateStatus();
    </script>
</body>
</html>
)";
    return html;
}