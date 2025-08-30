#include "../include/PIDController.h"

PIDController::PIDController() {
    kp = ki = kd = 0;
    setpoint = 0;
    outputMin = 0;
    outputMax = 255;
    
    integral = 0;
    previousError = 0;
    lastInput = 0;
    lastTime = 0;
    sampleTime = PID_SAMPLE_TIME;
    
    integralMax = 0;
    antiWindupEnabled = false;
    
    autoMode = false;
    reverseMode = false;
    
    derivativeFilter = 0;
    lastDerivative = 0;
    
    output = 0;
}

void PIDController::begin(float _kp, float _ki, float _kd) {
    setTunings(_kp, _ki, _kd);
    lastTime = millis();
    autoMode = true;
    reset();
}

float PIDController::compute(float input) {
    if (!autoMode) {
        return output;
    }
    
    unsigned long now = millis();
    unsigned long timeChange = now - lastTime;
    
    if (timeChange >= sampleTime) {
        // Calculate error
        float error = setpoint - input;
        if (reverseMode) {
            error = -error;
        }
        
        // Proportional term
        float pTerm = kp * error;
        
        // Integral term
        integral += (ki * error * timeChange / 1000.0);
        
        // Anti-windup
        if (antiWindupEnabled && integralMax > 0) {
            if (integral > integralMax) integral = integralMax;
            if (integral < -integralMax) integral = -integralMax;
        }
        
        // Derivative term
        float derivative = 0;
        if (timeChange > 0) {
            derivative = (input - lastInput) / (timeChange / 1000.0);
            
            // Apply derivative filter if enabled
            if (derivativeFilter > 0) {
                derivative = derivativeFilter * lastDerivative + 
                           (1 - derivativeFilter) * derivative;
                lastDerivative = derivative;
            }
        }
        float dTerm = -kd * derivative;  // Negative because we use input derivative
        
        // Calculate output
        output = pTerm + integral + dTerm;
        
        // Apply output limits
        if (output > outputMax) output = outputMax;
        if (output < outputMin) output = outputMin;
        
        // Anti-windup: prevent integral from growing when output is saturated
        if (antiWindupEnabled) {
            if ((output >= outputMax && error > 0) || 
                (output <= outputMin && error < 0)) {
                // Remove the integral contribution that was just added
                integral -= (ki * error * timeChange / 1000.0);
            }
        }
        
        // Store values for next iteration
        lastInput = input;
        previousError = error;
        lastTime = now;
    }
    
    return output;
}

void PIDController::setSetpoint(float sp) {
    setpoint = sp;
}

void PIDController::setTunings(float _kp, float _ki, float _kd) {
    if (_kp < 0 || _ki < 0 || _kd < 0) return;
    
    kp = _kp;
    ki = _ki;
    kd = _kd;
}

void PIDController::setOutputLimits(float min, float max) {
    if (min >= max) return;
    
    outputMin = min;
    outputMax = max;
    
    // Apply limits to current output
    if (output > outputMax) output = outputMax;
    if (output < outputMin) output = outputMin;
    
    // Apply limits to integral for anti-windup
    if (integral > outputMax) integral = outputMax;
    if (integral < outputMin) integral = outputMin;
}

void PIDController::setSampleTime(unsigned long time) {
    if (time > 0) {
        sampleTime = time;
    }
}

void PIDController::setMode(bool automatic) {
    bool newMode = automatic;
    if (newMode && !autoMode) {
        // Switching to auto mode, initialize
        reset();
        lastTime = millis();
    }
    autoMode = newMode;
}

void PIDController::setReverse(bool reverse) {
    reverseMode = reverse;
}

void PIDController::enableAntiWindup(bool enable, float maxIntegral) {
    antiWindupEnabled = enable;
    integralMax = maxIntegral;
}

void PIDController::setDerivativeFilter(float alpha) {
    if (alpha >= 0 && alpha <= 1) {
        derivativeFilter = alpha;
    }
}

void PIDController::reset() {
    integral = 0;
    previousError = 0;
    lastDerivative = 0;
}