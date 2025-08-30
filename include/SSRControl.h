#ifndef SSR_CONTROL_H
#define SSR_CONTROL_H

#include <Arduino.h>
#include "Config.h"

class SSRControl {
private:
    uint8_t ssrPin;
    unsigned long windowStartTime;
    unsigned long windowSize;
    float powerPercentage;
    float onTime;
    bool enabled;
    bool safetyLock;
    
    // PWM parameters for fine control
    bool usePWM;
    int pwmChannel;
    int pwmFrequency;
    int pwmResolution;
    
public:
    SSRControl();
    
    void begin();
    void begin(uint8_t pin);
    void update();
    
    void setPower(float power);  // 0-100 or 0-windowSize
    void setWindowSize(unsigned long size);
    void enable(bool state);
    void setSafetyLock(bool lock);
    
    float getPowerPercentage();
    bool isEnabled();
    bool isSafetyLocked();
    
    void emergencyStop();
    void test();
    
private:
    void setPinState(bool state);
};

#endif // SSR_CONTROL_H