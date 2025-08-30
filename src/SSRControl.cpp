#include "../include/SSRControl.h"

SSRControl::SSRControl() {
    ssrPin = SSR_PIN;
    windowStartTime = 0;
    windowSize = PID_WINDOW_SIZE;
    powerPercentage = 0;
    onTime = 0;
    enabled = false;
    safetyLock = false;
    usePWM = false;
    pwmChannel = 0;
    pwmFrequency = 5000;
    pwmResolution = 8;
}

void SSRControl::begin() {
    begin(ssrPin);
}

void SSRControl::begin(uint8_t pin) {
    ssrPin = pin;
    pinMode(ssrPin, OUTPUT);
    digitalWrite(ssrPin, LOW);
    windowStartTime = millis();
    enabled = true;
    
    DEBUG_PRINTLN(F("SSR Control initialized"));
}

void SSRControl::update() {
    if (!enabled || safetyLock) {
        setPinState(false);
        return;
    }
    
    unsigned long now = millis();
    
    // Check if we need to shift the window
    if (now - windowStartTime >= windowSize) {
        windowStartTime += windowSize;
    }
    
    // Time-proportional control
    if (onTime > 0 && (now - windowStartTime) < onTime) {
        setPinState(true);
    } else {
        setPinState(false);
    }
}

void SSRControl::setPower(float power) {
    // Accept input as either percentage (0-100) or raw value (0-windowSize)
    if (power <= 100) {
        // Treat as percentage
        powerPercentage = constrain(power, 0, 100);
        onTime = (windowSize * powerPercentage) / 100.0;
    } else {
        // Treat as raw value
        onTime = constrain(power, 0, windowSize);
        powerPercentage = (onTime * 100.0) / windowSize;
    }
}

void SSRControl::setWindowSize(unsigned long size) {
    windowSize = size;
    // Recalculate onTime based on current percentage
    onTime = (windowSize * powerPercentage) / 100.0;
}

void SSRControl::enable(bool state) {
    enabled = state;
    if (!enabled) {
        setPinState(false);
    }
}

void SSRControl::setSafetyLock(bool lock) {
    safetyLock = lock;
    if (safetyLock) {
        setPinState(false);
        DEBUG_PRINTLN(F("SSR Safety lock engaged"));
    }
}

float SSRControl::getPowerPercentage() {
    return powerPercentage;
}

bool SSRControl::isEnabled() {
    return enabled;
}

bool SSRControl::isSafetyLocked() {
    return safetyLock;
}

void SSRControl::emergencyStop() {
    setSafetyLock(true);
    setPinState(false);
    powerPercentage = 0;
    onTime = 0;
    DEBUG_PRINTLN(F("EMERGENCY STOP ACTIVATED"));
}

void SSRControl::test() {
    if (safetyLock) return;
    
    DEBUG_PRINTLN(F("Testing SSR..."));
    
    // Quick test pattern
    for (int i = 0; i < 3; i++) {
        setPinState(true);
        delay(500);
        setPinState(false);
        delay(500);
    }
    
    DEBUG_PRINTLN(F("SSR test complete"));
}

void SSRControl::setPinState(bool state) {
    digitalWrite(ssrPin, state ? HIGH : LOW);
}