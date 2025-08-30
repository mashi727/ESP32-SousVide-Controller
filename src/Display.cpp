#include "../include/Display.h"

Display::Display() {
    oled = nullptr;
    displayEnabled = false;
    lastUpdateTime = 0;
    needsRedraw = true;
    lastState = STATE_IDLE;
    animationFrame = 0;
    animationTimer = 0;
}

Display::~Display() {
    if (oled != nullptr) {
        delete oled;
    }
}

bool Display::begin() {
    // Initialize I2C
    Wire.begin(OLED_SDA, OLED_SCL);
    
    // Create OLED object
    oled = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    
    // Initialize display
    if (!oled->begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        DEBUG_PRINTLN(F("SSD1306 allocation failed"));
        return false;
    }
    
    // Clear display
    oled->clearDisplay();
    oled->setTextColor(SSD1306_WHITE);
    oled->setTextSize(1);
    oled->display();
    
    displayEnabled = true;
    return true;
}

void Display::clear() {
    oled->clearDisplay();
    oled->display();
}

void Display::update() {
    if (!displayEnabled) return;
    
    unsigned long currentTime = millis();
    if (currentTime - animationTimer >= 500) {
        animationTimer = currentTime;
        animationFrame = (animationFrame + 1) % 4;
    }
}

void Display::showStartupScreen() {
    oled->clearDisplay();
    
    // Draw logo or title
    oled->setTextSize(2);
    centerText("SOUS VIDE", 10);
    
    oled->setTextSize(1);
    centerText("Controller v1.0", 35);
    
    // Draw progress bar
    drawProgressBar(20, 50, 88, 8, 100);
    
    oled->display();
}

void Display::showIdleScreen(float currentTemp) {
    oled->clearDisplay();
    
    drawHeader("READY");
    
    // Display current temperature
    oled->setTextSize(2);
    String tempStr = formatTemperature(currentTemp);
    centerText(tempStr.c_str(), 25);
    
    oled->setTextSize(1);
    centerText("Press to start", 50);
    
    oled->display();
}

void Display::showSetupTempScreen(float targetTemp, float currentTemp) {
    oled->clearDisplay();
    
    drawHeader("SET TEMPERATURE");
    
    // Show target temperature (large)
    oled->setTextSize(2);
    String targetStr = formatTemperature(targetTemp);
    centerText(targetStr.c_str(), 20);
    
    // Show current temperature (small)
    oled->setTextSize(1);
    String currentStr = "Current: " + formatTemperature(currentTemp);
    centerText(currentStr.c_str(), 45);
    
    drawFooter("Turn to adjust");
    
    oled->display();
}

void Display::showSetupTimeScreen(unsigned long cookingTime, float currentTemp) {
    oled->clearDisplay();
    
    drawHeader("SET TIME");
    
    // Show cooking time
    oled->setTextSize(2);
    String timeStr = formatTime(cookingTime);
    centerText(timeStr.c_str(), 20);
    
    // Show current temperature
    oled->setTextSize(1);
    String tempStr = "Temp: " + formatTemperature(currentTemp);
    centerText(tempStr.c_str(), 45);
    
    drawFooter("Turn to adjust");
    
    oled->display();
}

void Display::showPreheatScreen(float currentTemp, float targetTemp) {
    oled->clearDisplay();
    
    drawHeader("PREHEATING");
    
    // Current temperature
    oled->setTextSize(1);
    oled->setCursor(10, 20);
    oled->print("Current:");
    oled->setTextSize(2);
    oled->setCursor(60, 16);
    oled->print(formatTemperature(currentTemp));
    
    // Target temperature
    oled->setTextSize(1);
    oled->setCursor(10, 38);
    oled->print("Target:");
    oled->setCursor(60, 38);
    oled->print(formatTemperature(targetTemp));
    
    // Progress bar
    float progress = (currentTemp / targetTemp) * 100;
    if (progress > 100) progress = 100;
    drawProgressBar(10, 52, 108, 6, progress);
    
    oled->display();
}

void Display::showCookingScreen(float currentTemp, float targetTemp, unsigned long remainingTime, float power) {
    oled->clearDisplay();
    
    // Header with time remaining
    oled->setTextSize(1);
    oled->setCursor(0, 0);
    oled->print("COOKING");
    oled->setCursor(70, 0);
    oled->print(formatTime(remainingTime));
    
    // Temperatures
    oled->setCursor(0, 16);
    oled->print("Temp:");
    oled->setTextSize(2);
    oled->setCursor(35, 12);
    oled->print(formatTemperature(currentTemp));
    
    oled->setTextSize(1);
    oled->setCursor(0, 32);
    oled->print("Set:");
    oled->setCursor(35, 32);
    oled->print(formatTemperature(targetTemp));
    
    // Power indicator
    oled->setCursor(0, 48);
    oled->print("Power:");
    drawProgressBar(40, 48, 80, 6, power);
    oled->setCursor(95, 48);
    oled->print(String(int(power)) + "%");
    
    // Heating indicator
    if (power > 0) {
        showHeatingIndicator(true);
    }
    
    oled->display();
}

void Display::showFinishedScreen() {
    oled->clearDisplay();
    
    oled->setTextSize(2);
    centerText("DONE!", 15);
    
    oled->setTextSize(1);
    centerText("Cooking complete", 40);
    
    // Blinking indicator
    if (animationFrame % 2 == 0) {
        centerText("Press to dismiss", 52);
    }
    
    oled->display();
}

void Display::showErrorScreen(ErrorCode error) {
    oled->clearDisplay();
    
    drawHeader("ERROR");
    
    oled->setTextSize(1);
    String errorMsg;
    
    switch(error) {
        case ERROR_SENSOR_DISCONNECTED:
            errorMsg = "Sensor disconnected";
            break;
        case ERROR_OVERTEMPERATURE:
            errorMsg = "Over temperature";
            break;
        case ERROR_UNDERTEMPERATURE:
            errorMsg = "Under temperature";
            break;
        case ERROR_PID_FAILURE:
            errorMsg = "PID control failure";
            break;
        case ERROR_SSR_FAILURE:
            errorMsg = "SSR failure";
            break;
        case ERROR_WIFI_CONNECTION:
            errorMsg = "WiFi connection lost";
            break;
        case ERROR_MEMORY_FULL:
            errorMsg = "Memory full";
            break;
        default:
            errorMsg = "Unknown error";
            break;
    }
    
    centerText(errorMsg.c_str(), 30);
    centerText("Check system", 45);
    
    oled->display();
}

void Display::showCalibrationScreen(float currentTemp, float offset) {
    oled->clearDisplay();
    
    drawHeader("CALIBRATION");
    
    oled->setTextSize(1);
    oled->setCursor(0, 20);
    oled->print("Raw: ");
    oled->print(currentTemp - offset, 1);
    oled->print(" C");
    
    oled->setCursor(0, 32);
    oled->print("Offset: ");
    oled->print(offset > 0 ? "+" : "");
    oled->print(offset, 1);
    oled->print(" C");
    
    oled->setCursor(0, 44);
    oled->print("Actual: ");
    oled->print(currentTemp, 1);
    oled->print(" C");
    
    drawFooter("Turn to adjust");
    
    oled->display();
}

void Display::showWiFiConfigScreen(const char* ssid, const char* ip) {
    oled->clearDisplay();
    
    drawHeader("WiFi CONFIG");
    
    oled->setTextSize(1);
    oled->setCursor(0, 20);
    oled->print("SSID: ");
    oled->print(ssid);
    
    oled->setCursor(0, 32);
    oled->print("IP: ");
    oled->print(ip);
    
    oled->setCursor(0, 50);
    oled->print("http://");
    oled->print(ip);
    
    oled->display();
}

void Display::updateScreen(SystemState state, float currentTemp, float targetTemp, 
                          unsigned long totalTime, unsigned long remainingTime, float power) {
    if (state != lastState) {
        needsRedraw = true;
        lastState = state;
    }
    
    switch(state) {
        case STATE_IDLE:
            showIdleScreen(currentTemp);
            break;
        case STATE_SETUP_TEMP:
            showSetupTempScreen(targetTemp, currentTemp);
            break;
        case STATE_SETUP_TIME:
            showSetupTimeScreen(totalTime, currentTemp);
            break;
        case STATE_PREHEAT:
            showPreheatScreen(currentTemp, targetTemp);
            break;
        case STATE_COOKING:
            showCookingScreen(currentTemp, targetTemp, remainingTime, power);
            break;
        case STATE_FINISHED:
            showFinishedScreen();
            break;
        default:
            break;
    }
}

void Display::drawProgressBar(int x, int y, int width, int height, float percentage) {
    // Draw border
    oled->drawRect(x, y, width, height, SSD1306_WHITE);
    
    // Draw fill
    int fillWidth = (width - 2) * (percentage / 100.0);
    if (fillWidth > 0) {
        oled->fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

void Display::drawTemperature(int x, int y, float temp, bool large) {
    oled->setCursor(x, y);
    oled->setTextSize(large ? 2 : 1);
    oled->print(temp, 1);
    oled->print("C");
}

void Display::drawTime(int x, int y, unsigned long seconds, bool showHours) {
    oled->setCursor(x, y);
    oled->print(formatTime(seconds));
}

void Display::drawPowerIndicator(int x, int y, float power) {
    drawProgressBar(x, y, 40, 6, power);
}

void Display::drawWiFiStatus(bool connected) {
    // Draw WiFi icon in corner
    int x = SCREEN_WIDTH - 10;
    int y = 0;
    
    if (connected) {
        oled->drawLine(x + 4, y + 6, x + 4, y + 7, SSD1306_WHITE);
        oled->drawLine(x + 2, y + 4, x + 2, y + 7, SSD1306_WHITE);
        oled->drawLine(x + 6, y + 4, x + 6, y + 7, SSD1306_WHITE);
        oled->drawLine(x, y + 2, x, y + 7, SSD1306_WHITE);
        oled->drawLine(x + 8, y + 2, x + 8, y + 7, SSD1306_WHITE);
    }
}

void Display::showHeatingIndicator(bool active) {
    if (active && (animationFrame % 2 == 0)) {
        oled->fillCircle(SCREEN_WIDTH - 8, 8, 3, SSD1306_WHITE);
    }
}

void Display::showAlarmIndicator(bool active) {
    if (active) {
        // Draw bell icon
        int x = SCREEN_WIDTH - 20;
        int y = 0;
        oled->drawPixel(x + 3, y, SSD1306_WHITE);
        oled->drawLine(x + 2, y + 1, x + 4, y + 1, SSD1306_WHITE);
        oled->drawLine(x + 1, y + 2, x + 5, y + 2, SSD1306_WHITE);
        oled->drawLine(x + 1, y + 3, x + 5, y + 3, SSD1306_WHITE);
        oled->drawLine(x + 1, y + 4, x + 5, y + 4, SSD1306_WHITE);
        oled->drawLine(x + 2, y + 5, x + 4, y + 5, SSD1306_WHITE);
    }
}

void Display::setBrightness(uint8_t brightness) {
    // SSD1306 doesn't support brightness control via software
    // This would need to be implemented with PWM on the power pin
}

void Display::setRotation(uint8_t rotation) {
    oled->setRotation(rotation);
}

void Display::enableDisplay(bool enable) {
    displayEnabled = enable;
    if (!enable) {
        oled->clearDisplay();
        oled->display();
    }
}

bool Display::isEnabled() {
    return displayEnabled;
}

void Display::drawHeader(const char* title) {
    oled->setTextSize(1);
    oled->setCursor(0, 0);
    oled->print(title);
    oled->drawLine(0, 9, SCREEN_WIDTH, 9, SSD1306_WHITE);
}

void Display::drawFooter(const char* text) {
    oled->setTextSize(1);
    oled->setCursor(0, 56);
    oled->print(text);
}

void Display::centerText(const char* text, int y, int size) {
    oled->setTextSize(size);
    int16_t x1, y1;
    uint16_t w, h;
    oled->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    oled->setCursor((SCREEN_WIDTH - w) / 2, y);
    oled->print(text);
}

String Display::formatTime(unsigned long seconds) {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    
    char buffer[10];
    if (hours > 0) {
        sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, secs);
    } else {
        sprintf(buffer, "%02lu:%02lu", minutes, secs);
    }
    return String(buffer);
}

String Display::formatTemperature(float temp) {
    char buffer[10];
    sprintf(buffer, "%.1f", temp);
    return String(buffer) + "C";
}