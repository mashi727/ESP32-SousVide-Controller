#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"

class Display {
private:
    Adafruit_SSD1306* oled;
    bool displayEnabled;
    unsigned long lastUpdateTime;
    bool needsRedraw;
    SystemState lastState;
    
    // Animation variables
    int animationFrame;
    unsigned long animationTimer;
    
public:
    Display();
    ~Display();
    
    bool begin();
    void clear();
    void update();
    
    // Screen display methods
    void showStartupScreen();
    void showIdleScreen(float currentTemp);
    void showSetupTempScreen(float targetTemp, float currentTemp);
    void showSetupTimeScreen(unsigned long cookingTime, float currentTemp);
    void showPreheatScreen(float currentTemp, float targetTemp);
    void showCookingScreen(float currentTemp, float targetTemp, unsigned long remainingTime, float power);
    void showFinishedScreen();
    void showErrorScreen(ErrorCode error);
    void showCalibrationScreen(float currentTemp, float offset);
    void showWiFiConfigScreen(const char* ssid, const char* ip);
    
    // Update main screen based on state
    void updateScreen(SystemState state, float currentTemp, float targetTemp, 
                     unsigned long totalTime, unsigned long remainingTime, float power);
    
    // Utility display methods
    void drawProgressBar(int x, int y, int width, int height, float percentage);
    void drawTemperature(int x, int y, float temp, bool large = false);
    void drawTime(int x, int y, unsigned long seconds, bool showHours = true);
    void drawPowerIndicator(int x, int y, float power);
    void drawWiFiStatus(bool connected);
    void drawBatteryLevel(float voltage);  // For battery-powered versions
    
    // Graph display
    void drawTemperatureGraph(float* tempHistory, int historySize, float minTemp, float maxTemp);
    
    // Menu system
    void showMenu(const char* items[], int itemCount, int selectedIndex);
    void showConfirmDialog(const char* message);
    
    // Status indicators
    void showHeatingIndicator(bool active);
    void showAlarmIndicator(bool active);
    void showLoggingIndicator(bool active);
    
    // Display settings
    void setBrightness(uint8_t brightness);
    void setRotation(uint8_t rotation);
    void enableDisplay(bool enable);
    bool isEnabled();
    
private:
    void drawHeader(const char* title);
    void drawFooter(const char* text);
    void centerText(const char* text, int y, int size = 1);
    String formatTime(unsigned long seconds);
    String formatTemperature(float temp);
    void updateAnimation();
};

#endif // DISPLAY_H