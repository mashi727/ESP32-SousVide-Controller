#include "../include/StateMachine.h"

StateMachine::StateMachine() {
    currentState = STATE_IDLE;
    previousState = STATE_IDLE;
    
    cookingParams.targetTemperature = DEFAULT_TARGET_TEMP;
    cookingParams.cookingTime = DEFAULT_COOKING_TIME;
    cookingParams.pidKp = DEFAULT_KP;
    cookingParams.pidKi = DEFAULT_KI;
    cookingParams.pidKd = DEFAULT_KD;
    cookingParams.preHeatEnabled = true;
    cookingParams.alarmEnabled = true;
    
    cookingStartTime = 0;
    cookingEndTime = 0;
    stateChangeTime = 0;
    
    isPreheated = false;
    alarmActive = false;
    lastError = ERROR_NONE;
}

void StateMachine::begin() {
    changeState(STATE_IDLE);
    DEBUG_PRINTLN(F("State Machine initialized"));
}

void StateMachine::update(float currentTemp, Encoder& encoder) {
    // Check for errors first
    if (currentTemp == SENSOR_ERROR_TEMP) {
        setError(ERROR_SENSOR_DISCONNECTED);
    } else if (currentTemp > MAX_TEMP_LIMIT) {
        setError(ERROR_OVERTEMPERATURE);
    }
    
    // Handle state-specific logic
    switch (currentState) {
        case STATE_IDLE:
            handleIdleState(currentTemp, encoder);
            break;
        case STATE_SETUP_TEMP:
            handleSetupTempState(currentTemp, encoder);
            break;
        case STATE_SETUP_TIME:
            handleSetupTimeState(currentTemp, encoder);
            break;
        case STATE_PREHEAT:
            handlePreheatState(currentTemp, encoder);
            break;
        case STATE_COOKING:
            handleCookingState(currentTemp, encoder);
            break;
        case STATE_FINISHED:
            handleFinishedState(currentTemp, encoder);
            break;
        case STATE_ERROR:
            handleErrorState(currentTemp, encoder);
            break;
        default:
            break;
    }
    
    // Update alarm if needed
    if (cookingParams.alarmEnabled) {
        updateAlarm(currentTemp);
    }
}

void StateMachine::changeState(SystemState newState) {
    previousState = currentState;
    currentState = newState;
    stateChangeTime = millis();
    
    DEBUG_PRINT(F("State changed to: "));
    DEBUG_PRINTLN(newState);
}

unsigned long StateMachine::getRemainingTime() {
    if (currentState != STATE_COOKING) {
        return 0;
    }
    
    unsigned long elapsed = (millis() - cookingStartTime) / 1000;
    if (elapsed >= cookingParams.cookingTime) {
        return 0;
    }
    
    return cookingParams.cookingTime - elapsed;
}

unsigned long StateMachine::getElapsedTime() {
    if (cookingStartTime == 0) {
        return 0;
    }
    
    return (millis() - cookingStartTime) / 1000;
}

void StateMachine::setTargetTemperature(float temp) {
    cookingParams.targetTemperature = constrain(temp, MIN_TEMP, MAX_TEMP);
}

void StateMachine::setCookingTime(unsigned long time) {
    cookingParams.cookingTime = constrain(time, MIN_COOKING_TIME, MAX_COOKING_TIME);
}

void StateMachine::startCooking() {
    if (cookingParams.preHeatEnabled && !isPreheated) {
        changeState(STATE_PREHEAT);
    } else {
        cookingStartTime = millis();
        cookingEndTime = cookingStartTime + (cookingParams.cookingTime * 1000);
        changeState(STATE_COOKING);
    }
}

void StateMachine::stopCooking() {
    cookingStartTime = 0;
    cookingEndTime = 0;
    isPreheated = false;
    changeState(STATE_IDLE);
}

void StateMachine::pauseCooking() {
    // Store remaining time
    if (currentState == STATE_COOKING) {
        unsigned long remaining = getRemainingTime();
        cookingParams.cookingTime = remaining;
        changeState(STATE_IDLE);
    }
}

void StateMachine::resumeCooking() {
    if (currentState == STATE_IDLE && cookingParams.cookingTime > 0) {
        startCooking();
    }
}

void StateMachine::setError(ErrorCode error) {
    lastError = error;
    if (error != ERROR_NONE) {
        changeState(STATE_ERROR);
    }
}

void StateMachine::clearError() {
    lastError = ERROR_NONE;
    if (currentState == STATE_ERROR) {
        changeState(STATE_IDLE);
    }
}

void StateMachine::enableAlarm(bool enable) {
    cookingParams.alarmEnabled = enable;
    if (!enable) {
        alarmActive = false;
    }
}

void StateMachine::handleIdleState(float currentTemp, Encoder& encoder) {
    // Check for button press to start setup
    if (encoder.wasButtonPressed()) {
        changeState(STATE_SETUP_TEMP);
    }
    
    // Long press for calibration or WiFi config
    if (encoder.isLongPress()) {
        changeState(STATE_CALIBRATION);
    }
}

void StateMachine::handleSetupTempState(float currentTemp, Encoder& encoder) {
    // Adjust temperature with encoder
    if (encoder.hasChanged()) {
        int32_t change = encoder.getChange();
        float newTemp = cookingParams.targetTemperature + (change * TEMP_STEP);
        setTargetTemperature(newTemp);
    }
    
    // Button press to confirm and move to time setup
    if (encoder.wasButtonPressed()) {
        changeState(STATE_SETUP_TIME);
    }
    
    // Long press to go back
    if (encoder.isLongPress()) {
        changeState(STATE_IDLE);
    }
}

void StateMachine::handleSetupTimeState(float currentTemp, Encoder& encoder) {
    // Adjust time with encoder
    if (encoder.hasChanged()) {
        int32_t change = encoder.getChange();
        long newTime = cookingParams.cookingTime + (change * TIME_STEP);
        setCookingTime(newTime);
    }
    
    // Button press to start cooking
    if (encoder.wasButtonPressed()) {
        startCooking();
    }
    
    // Long press to go back
    if (encoder.isLongPress()) {
        changeState(STATE_SETUP_TEMP);
    }
}

void StateMachine::handlePreheatState(float currentTemp, Encoder& encoder) {
    // Check if target temperature reached
    if (checkTemperatureReached(currentTemp, cookingParams.targetTemperature)) {
        isPreheated = true;
        cookingStartTime = millis();
        cookingEndTime = cookingStartTime + (cookingParams.cookingTime * 1000);
        changeState(STATE_COOKING);
    }
    
    // Button press to skip preheat
    if (encoder.wasButtonPressed()) {
        isPreheated = true;
        cookingStartTime = millis();
        cookingEndTime = cookingStartTime + (cookingParams.cookingTime * 1000);
        changeState(STATE_COOKING);
    }
    
    // Long press to cancel
    if (encoder.isLongPress()) {
        stopCooking();
    }
}

void StateMachine::handleCookingState(float currentTemp, Encoder& encoder) {
    // Check if cooking time has elapsed
    if (getRemainingTime() == 0) {
        changeState(STATE_FINISHED);
        if (cookingParams.alarmEnabled) {
            alarmActive = true;
        }
    }
    
    // Button press to pause
    if (encoder.wasButtonPressed()) {
        pauseCooking();
    }
    
    // Long press to stop
    if (encoder.isLongPress()) {
        stopCooking();
    }
}

void StateMachine::handleFinishedState(float currentTemp, Encoder& encoder) {
    // Button press to dismiss
    if (encoder.wasButtonPressed()) {
        alarmActive = false;
        stopCooking();
    }
}

void StateMachine::handleErrorState(float currentTemp, Encoder& encoder) {
    // Check if error condition is resolved
    if (currentTemp != SENSOR_ERROR_TEMP && currentTemp < MAX_TEMP_LIMIT) {
        clearError();
    }
    
    // Button press to acknowledge error
    if (encoder.wasButtonPressed()) {
        clearError();
    }
}

bool StateMachine::checkTemperatureReached(float current, float target, float tolerance) {
    return abs(current - target) <= tolerance;
}

void StateMachine::updateAlarm(float currentTemp) {
    if (currentState == STATE_COOKING) {
        // Check for temperature deviation
        float deviation = abs(currentTemp - cookingParams.targetTemperature);
        if (deviation > TEMP_ALARM_THRESHOLD) {
            alarmActive = true;
        } else {
            alarmActive = false;
        }
    }
}