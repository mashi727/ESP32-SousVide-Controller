#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include "Config.h"

class Encoder {
public:
    enum ButtonState {
        BUTTON_IDLE,
        BUTTON_PRESSED,
        BUTTON_RELEASED,
        BUTTON_LONG_PRESS,
        BUTTON_DOUBLE_CLICK
    };
    
    enum Direction {
        DIRECTION_NONE,
        DIRECTION_CW,
        DIRECTION_CCW
    };

private:
    // Pin definitions
    uint8_t pinA;
    uint8_t pinB;
    uint8_t pinButton;
    
    // Encoder state
    volatile int32_t position;
    volatile int32_t lastPosition;
    int32_t minValue;
    int32_t maxValue;
    bool wrapEnabled;
    
    // Button state
    ButtonState buttonState;
    unsigned long buttonPressTime;
    unsigned long lastButtonPressTime;
    unsigned long lastDebounceTime;
    bool lastButtonState;
    bool buttonPressed;
    int clickCount;
    
    // Rotation state
    volatile uint8_t encoderState;
    static const int8_t encoderTable[16];
    
    // Acceleration
    unsigned long lastRotationTime;
    int accelerationFactor;
    bool accelerationEnabled;
    
    // ISR handlers
    static Encoder* instance;
    static void IRAM_ATTR handleInterrupt();
    void IRAM_ATTR updateEncoder();
    
public:
    Encoder();
    ~Encoder();
    
    void begin();
    void begin(uint8_t pinA, uint8_t pinB, uint8_t pinButton);
    void update();
    
    // Position management
    int32_t getPosition();
    void setPosition(int32_t pos);
    void setRange(int32_t min, int32_t max);
    void enableWrap(bool enable);
    void reset();
    
    // Movement detection
    bool hasChanged();
    int32_t getChange();
    Direction getDirection();
    
    // Button management
    ButtonState getButtonState();
    bool isButtonPressed();
    bool wasButtonPressed();
    bool wasButtonReleased();
    bool isLongPress();
    bool isDoubleClick();
    void clearButtonState();
    
    // Acceleration
    void enableAcceleration(bool enable);
    void setAccelerationFactor(int factor);
    int getAccelerationMultiplier();
    
    // Value mapping
    float mapToFloat(float min, float max, float step = 1.0);
    int mapToInt(int min, int max, int step = 1);
    
    // Utility
    void attachInterrupts();
    void detachInterrupts();
    void setDebounceTime(unsigned long time);
};

#endif // ENCODER_H