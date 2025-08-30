#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

class PIDController {
private:
    // PID parameters
    float kp, ki, kd;
    float setpoint;
    float outputMin, outputMax;
    
    // PID state variables
    float integral;
    float previousError;
    float lastInput;
    unsigned long lastTime;
    unsigned long sampleTime;
    
    // Anti-windup
    float integralMax;
    bool antiWindupEnabled;
    
    // Mode
    bool autoMode;
    bool reverseMode;
    
    // Derivative filter
    float derivativeFilter;
    float lastDerivative;
    
    // Output
    float output;
    
public:
    PIDController();
    
    void begin(float kp, float ki, float kd);
    float compute(float input);
    
    // Setters
    void setSetpoint(float sp);
    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float min, float max);
    void setSampleTime(unsigned long time);
    void setMode(bool automatic);
    void setReverse(bool reverse);
    
    // Getters
    float getSetpoint() { return setpoint; }
    float getKp() { return kp; }
    float getKi() { return ki; }
    float getKd() { return kd; }
    float getOutput() { return output; }
    bool isAutoMode() { return autoMode; }
    
    // Advanced features
    void enableAntiWindup(bool enable, float maxIntegral = 0);
    void setDerivativeFilter(float alpha);
    void reset();
    
    // Auto-tuning support
    struct TuningParameters {
        float kp, ki, kd;
        float overshoot;
        float settlingTime;
    };
    TuningParameters autoTune(float input, float target);
};

#endif // PID_CONTROLLER_H