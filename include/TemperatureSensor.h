#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"

class TemperatureSensor {
private:
    OneWire* oneWire;
    DallasTemperature* sensors;
    DeviceAddress sensorAddress;
    float lastTemperature;
    float temperatureOffset;  // Calibration offset
    unsigned long lastReadTime;
    bool sensorFound;
    bool isCalibrated;
    
    // Moving average filter
    static const int FILTER_SIZE = 10;
    float tempHistory[FILTER_SIZE];
    int historyIndex;
    bool historyFilled;
    
public:
    TemperatureSensor();
    ~TemperatureSensor();
    
    bool begin();
    void update();
    float getTemperature();
    float getRawTemperature();
    float getFilteredTemperature();
    bool isConnected();
    bool hasError();
    
    // Calibration methods
    void setCalibrationOffset(float offset);
    float getCalibrationOffset();
    void calibrateToReference(float referenceTemp);
    
    // Sensor management
    int getSensorCount();
    void printAddress(DeviceAddress deviceAddress);
    bool getSensorAddress(DeviceAddress& address, int index = 0);
    void setResolution(uint8_t resolution);
    uint8_t getResolution();
    
    // Statistics
    float getMinTemperature();
    float getMaxTemperature();
    float getAverageTemperature();
    void resetStatistics();
    
private:
    void updateMovingAverage(float newTemp);
    float calculateMovingAverage();
    bool validateReading(float temp);
};

#endif // TEMPERATURE_SENSOR_H