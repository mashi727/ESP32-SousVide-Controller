#include "../include/Encoder.h"

Encoder* Encoder::instance = nullptr;

// Encoder state table for reliable reading
const int8_t Encoder::encoderTable[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0
};

Encoder::Encoder() {
    pinA = ENCODER_PIN_A;
    pinB = ENCODER_PIN_B;
    pinButton = ENCODER_BUTTON_PIN;
    
    position = 0;
    lastPosition = 0;
    minValue = INT32_MIN;
    maxValue = INT32_MAX;
    wrapEnabled = false;
    
    buttonState = BUTTON_IDLE;
    buttonPressTime = 0;
    lastButtonPressTime = 0;
    lastDebounceTime = 0;
    lastButtonState = HIGH;
    buttonPressed = false;
    clickCount = 0;
    
    encoderState = 0;
    lastRotationTime = 0;
    accelerationFactor = 1;
    accelerationEnabled = false;
    
    instance = this;
}

Encoder::~Encoder() {
    detachInterrupts();
    instance = nullptr;
}

void Encoder::begin() {
    begin(pinA, pinB, pinButton);
}

void Encoder::begin(uint8_t _pinA, uint8_t _pinB, uint8_t _pinButton) {
    pinA = _pinA;
    pinB = _pinB;
    pinButton = _pinButton;
    
    // Configure encoder pins
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(pinButton, INPUT_PULLUP);
    
    // Read initial state
    uint8_t a = digitalRead(pinA);
    uint8_t b = digitalRead(pinB);
    encoderState = (a << 1) | b;
    
    // Attach interrupts
    attachInterrupts();
    
    DEBUG_PRINTLN(F("Encoder initialized"));
}

void Encoder::update() {
    // Update button state
    bool currentButtonState = digitalRead(pinButton);
    unsigned long currentTime = millis();
    
    // Debounce button
    if (currentButtonState != lastButtonState) {
        lastDebounceTime = currentTime;
    }
    
    if ((currentTime - lastDebounceTime) > BUTTON_DEBOUNCE_TIME) {
        // Button state has been stable
        if (currentButtonState == LOW && !buttonPressed) {
            // Button pressed
            buttonPressed = true;
            buttonPressTime = currentTime;
            buttonState = BUTTON_PRESSED;
            
            // Check for double click
            if (currentTime - lastButtonPressTime < 500) {
                clickCount++;
                if (clickCount >= 2) {
                    buttonState = BUTTON_DOUBLE_CLICK;
                    clickCount = 0;
                }
            } else {
                clickCount = 1;
            }
            
            lastButtonPressTime = currentTime;
            
        } else if (currentButtonState == HIGH && buttonPressed) {
            // Button released
            buttonPressed = false;
            
            if (currentTime - buttonPressTime >= BUTTON_LONG_PRESS_TIME) {
                buttonState = BUTTON_LONG_PRESS;
            } else {
                buttonState = BUTTON_RELEASED;
            }
        }
        
        // Check for long press while button is held
        if (buttonPressed && (currentTime - buttonPressTime >= BUTTON_LONG_PRESS_TIME)) {
            if (buttonState != BUTTON_LONG_PRESS) {
                buttonState = BUTTON_LONG_PRESS;
            }
        }
    }
    
    lastButtonState = currentButtonState;
    
    // Update acceleration
    if (accelerationEnabled && hasChanged()) {
        unsigned long timeDiff = currentTime - lastRotationTime;
        if (timeDiff < 50) {
            accelerationFactor = 10;
        } else if (timeDiff < 100) {
            accelerationFactor = 5;
        } else if (timeDiff < 200) {
            accelerationFactor = 2;
        } else {
            accelerationFactor = 1;
        }
        lastRotationTime = currentTime;
    }
}

void IRAM_ATTR Encoder::handleInterrupt() {
    if (instance != nullptr) {
        instance->updateEncoder();
    }
}

void IRAM_ATTR Encoder::updateEncoder() {
    uint8_t a = digitalRead(pinA);
    uint8_t b = digitalRead(pinB);
    uint8_t newState = (a << 1) | b;
    uint8_t tableIndex = (encoderState << 2) | newState;
    int8_t motion = encoderTable[tableIndex];
    
    if (motion != 0) {
        position += motion;
        
        // Apply range limits
        if (!wrapEnabled) {
            if (position < minValue) position = minValue;
            if (position > maxValue) position = maxValue;
        } else {
            // Wrap around
            if (position < minValue) position = maxValue;
            if (position > maxValue) position = minValue;
        }
    }
    
    encoderState = newState;
}

int32_t Encoder::getPosition() {
    return position / ENCODER_STEPS_PER_NOTCH;
}

void Encoder::setPosition(int32_t pos) {
    position = pos * ENCODER_STEPS_PER_NOTCH;
    lastPosition = position;
}

void Encoder::setRange(int32_t min, int32_t max) {
    minValue = min * ENCODER_STEPS_PER_NOTCH;
    maxValue = max * ENCODER_STEPS_PER_NOTCH;
}

void Encoder::enableWrap(bool enable) {
    wrapEnabled = enable;
}

void Encoder::reset() {
    position = 0;
    lastPosition = 0;
    buttonState = BUTTON_IDLE;
    clickCount = 0;
}

bool Encoder::hasChanged() {
    return position != lastPosition;
}

int32_t Encoder::getChange() {
    int32_t change = (position - lastPosition) / ENCODER_STEPS_PER_NOTCH;
    if (accelerationEnabled && change != 0) {
        change *= accelerationFactor;
    }
    lastPosition = position;
    return change;
}

Encoder::Direction Encoder::getDirection() {
    int32_t diff = position - lastPosition;
    if (diff > 0) return DIRECTION_CW;
    if (diff < 0) return DIRECTION_CCW;
    return DIRECTION_NONE;
}

Encoder::ButtonState Encoder::getButtonState() {
    return buttonState;
}

bool Encoder::isButtonPressed() {
    return buttonPressed;
}

bool Encoder::wasButtonPressed() {
    if (buttonState == BUTTON_PRESSED) {
        buttonState = BUTTON_IDLE;
        return true;
    }
    return false;
}

bool Encoder::wasButtonReleased() {
    if (buttonState == BUTTON_RELEASED) {
        buttonState = BUTTON_IDLE;
        return true;
    }
    return false;
}

bool Encoder::isLongPress() {
    if (buttonState == BUTTON_LONG_PRESS) {
        buttonState = BUTTON_IDLE;
        return true;
    }
    return false;
}

bool Encoder::isDoubleClick() {
    if (buttonState == BUTTON_DOUBLE_CLICK) {
        buttonState = BUTTON_IDLE;
        return true;
    }
    return false;
}

void Encoder::clearButtonState() {
    buttonState = BUTTON_IDLE;
}

void Encoder::enableAcceleration(bool enable) {
    accelerationEnabled = enable;
    if (!enable) {
        accelerationFactor = 1;
    }
}

void Encoder::setAccelerationFactor(int factor) {
    accelerationFactor = factor;
}

int Encoder::getAccelerationMultiplier() {
    return accelerationFactor;
}

float Encoder::mapToFloat(float min, float max, float step) {
    int32_t pos = getPosition();
    int32_t steps = (max - min) / step;
    if (pos < 0) pos = 0;
    if (pos > steps) pos = steps;
    return min + (pos * step);
}

int Encoder::mapToInt(int min, int max, int step) {
    int32_t pos = getPosition();
    int32_t steps = (max - min) / step;
    if (pos < 0) pos = 0;
    if (pos > steps) pos = steps;
    return min + (pos * step);
}

void Encoder::attachInterrupts() {
    attachInterrupt(digitalPinToInterrupt(pinA), handleInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinB), handleInterrupt, CHANGE);
}

void Encoder::detachInterrupts() {
    detachInterrupt(digitalPinToInterrupt(pinA));
    detachInterrupt(digitalPinToInterrupt(pinB));
}

void Encoder::setDebounceTime(unsigned long time) {
    // This would allow runtime configuration of debounce time
    // For now, we use the compile-time constant BUTTON_DEBOUNCE_TIME
}