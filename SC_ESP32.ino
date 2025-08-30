/**
 * ESP32 Sous Vide Controller
 * 
 * A modular and extensible temperature controller for sous vide cooking
 * using ESP32 microcontroller with PID control.
 * 
 * @author Your Name
 * @version 1.0.0
 * @license MIT
 */

#include <Arduino.h>
#include "include/Config.h"
#include "include/TemperatureSensor.h"
#include "include/Display.h"
#include "include/Encoder.h"
#include "include/PIDController.h"
#include "include/SSRControl.h"
#include "include/StateMachine.h"
#include "include/DataLogger.h"
#include "include/WebInterface.h"

// Global objects
TemperatureSensor tempSensor;
Display display;
Encoder encoder;
PIDController pidController;
SSRControl ssrControl;
StateMachine stateMachine;
DataLogger dataLogger;
WebInterface webInterface;

// Global variables
unsigned long lastUpdateTime = 0;
unsigned long lastLogTime = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("ESP32 Sous Vide Controller Starting..."));
    
    // Initialize hardware components
    if (!tempSensor.begin()) {
        Serial.println(F("ERROR: Temperature sensor initialization failed"));
    }
    
    if (!display.begin()) {
        Serial.println(F("ERROR: Display initialization failed"));
    }
    
    encoder.begin();
    ssrControl.begin();
    
    // Initialize PID controller with default parameters
    pidController.begin(DEFAULT_KP, DEFAULT_KI, DEFAULT_KD);
    pidController.setOutputLimits(0, PID_WINDOW_SIZE);
    pidController.setSetpoint(DEFAULT_TARGET_TEMP);
    
    // Initialize state machine
    stateMachine.begin();
    
    // Initialize data logger
    if (ENABLE_DATA_LOGGING) {
        dataLogger.begin();
    }
    
    // Initialize WiFi and web interface
    if (ENABLE_WIFI) {
        webInterface.begin();
    }
    
    // Display startup message
    display.showStartupScreen();
    delay(2000);
    
    Serial.println(F("Initialization complete"));
}

void loop() {
    unsigned long currentTime = millis();
    
    // Update temperature reading
    tempSensor.update();
    float currentTemp = tempSensor.getTemperature();
    
    // Update encoder state
    encoder.update();
    
    // Process state machine
    stateMachine.update(currentTemp, encoder);
    
    // Get current cooking parameters from state machine
    CookingParameters params = stateMachine.getCookingParameters();
    
    // Update PID controller if cooking
    if (stateMachine.getCurrentState() == STATE_COOKING) {
        pidController.setSetpoint(params.targetTemperature);
        float output = pidController.compute(currentTemp);
        ssrControl.setPower(output);
    } else {
        ssrControl.setPower(0);  // Turn off heater when not cooking
    }
    
    // Update display (limit refresh rate)
    if (currentTime - lastUpdateTime >= DISPLAY_UPDATE_INTERVAL) {
        lastUpdateTime = currentTime;
        
        display.updateScreen(
            stateMachine.getCurrentState(),
            currentTemp,
            params.targetTemperature,
            params.cookingTime,
            stateMachine.getRemainingTime(),
            ssrControl.getPowerPercentage()
        );
    }
    
    // Log data if enabled
    if (ENABLE_DATA_LOGGING && stateMachine.getCurrentState() == STATE_COOKING) {
        if (currentTime - lastLogTime >= DATA_LOG_INTERVAL) {
            lastLogTime = currentTime;
            
            dataLogger.logData(
                currentTemp,
                params.targetTemperature,
                ssrControl.getPowerPercentage(),
                stateMachine.getRemainingTime()
            );
        }
    }
    
    // Update web interface if enabled
    if (ENABLE_WIFI) {
        webInterface.update(
            stateMachine.getCurrentState(),
            currentTemp,
            params.targetTemperature,
            stateMachine.getRemainingTime(),
            ssrControl.getPowerPercentage()
        );
    }
    
    // Update SSR control (needs to be called frequently for PWM)
    ssrControl.update();
    
    // Small delay to prevent watchdog issues
    delay(10);
}