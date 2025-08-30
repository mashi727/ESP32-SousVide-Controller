#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "Config.h"
#include "Encoder.h"

class StateMachine {
private:
    SystemState currentState;
    SystemState previousState;
    
    CookingParameters cookingParams;
    unsigned long cookingStartTime;
    unsigned long cookingEndTime;
    unsigned long stateChangeTime;
    
    bool isPreheated;
    bool alarmActive;
    ErrorCode lastError;
    
public:
    StateMachine();
    
    void begin();
    void update(float currentTemp, Encoder& encoder);
    
    SystemState getCurrentState() { return currentState; }
    SystemState getPreviousState() { return previousState; }
    CookingParameters getCookingParameters() { return cookingParams; }
    
    void changeState(SystemState newState);
    unsigned long getRemainingTime();
    unsigned long getElapsedTime();
    
    void setTargetTemperature(float temp);
    void setCookingTime(unsigned long time);
    void startCooking();
    void stopCooking();
    void pauseCooking();
    void resumeCooking();
    
    void setError(ErrorCode error);
    void clearError();
    bool hasError() { return lastError != ERROR_NONE; }
    
    void enableAlarm(bool enable);
    bool isAlarmActive() { return alarmActive; }
    
private:
    void handleIdleState(float currentTemp, Encoder& encoder);
    void handleSetupTempState(float currentTemp, Encoder& encoder);
    void handleSetupTimeState(float currentTemp, Encoder& encoder);
    void handlePreheatState(float currentTemp, Encoder& encoder);
    void handleCookingState(float currentTemp, Encoder& encoder);
    void handleFinishedState(float currentTemp, Encoder& encoder);
    void handleErrorState(float currentTemp, Encoder& encoder);
    
    bool checkTemperatureReached(float current, float target, float tolerance = 1.0);
    void updateAlarm(float currentTemp);
};

#endif // STATE_MACHINE_H