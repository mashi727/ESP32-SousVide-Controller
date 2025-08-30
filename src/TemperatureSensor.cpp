#include "../include/TemperatureSensor.h"

TemperatureSensor::TemperatureSensor() {
    oneWire = nullptr;
    sensors = nullptr;
    lastTemperature = 0.0;
    temperatureOffset = 0.0;
    lastReadTime = 0;
    sensorFound = false;
    isCalibrated = false;
    historyIndex = 0;
    historyFilled = false;
    
    // Initialize temperature history
    for (int i = 0; i < FILTER_SIZE; i++) {
        tempHistory[i] = 0.0;
    }
}

TemperatureSensor::~TemperatureSensor() {
    if (sensors != nullptr) {
        delete sensors;
    }
    if (oneWire != nullptr) {
        delete oneWire;
    }
}

bool TemperatureSensor::begin() {
    // Initialize OneWire and DallasTemperature
    oneWire = new OneWire(ONE_WIRE_BUS);
    sensors = new DallasTemperature(oneWire);
    
    // Start the library
    sensors->begin();
    
    // Check if any sensors are found
    int deviceCount = sensors->getDeviceCount();
    DEBUG_PRINT(F("Found "));
    DEBUG_PRINT(deviceCount);
    DEBUG_PRINTLN(F(" temperature sensor(s)"));
    
    if (deviceCount == 0) {
        DEBUG_PRINTLN(F("ERROR: No temperature sensors found!"));
        return false;
    }
    
    // Get the address of the first sensor
    if (!sensors->getAddress(sensorAddress, 0)) {
        DEBUG_PRINTLN(F("ERROR: Unable to find address for sensor 0"));
        return false;
    }
    
    // Print sensor address
    DEBUG_PRINT(F("Sensor 0 Address: "));
    printAddress(sensorAddress);
    DEBUG_PRINTLN();
    
    // Set resolution
    sensors->setResolution(sensorAddress, TEMP_RESOLUTION);
    
    // Set to async mode for non-blocking reads
    sensors->setWaitForConversion(false);
    
    // Request first temperature
    sensors->requestTemperatures();
    lastReadTime = millis();
    
    sensorFound = true;
    return true;
}

void TemperatureSensor::update() {
    unsigned long currentTime = millis();
    
    // Check if it's time to read temperature
    if (currentTime - lastReadTime >= TEMP_READ_INTERVAL) {
        // Get temperature from last request
        float tempC = sensors->getTempC(sensorAddress);
        
        // Validate reading
        if (validateReading(tempC)) {
            // Apply calibration offset
            float calibratedTemp = tempC + temperatureOffset;
            
            // Update moving average
            updateMovingAverage(calibratedTemp);
            
            // Store last valid temperature
            lastTemperature = calibratedTemp;
        } else {
            DEBUG_PRINTLN(F("WARNING: Invalid temperature reading"));
        }
        
        // Request next temperature (non-blocking)
        sensors->requestTemperatures();
        lastReadTime = currentTime;
    }
}

float TemperatureSensor::getTemperature() {
    return lastTemperature;
}

float TemperatureSensor::getRawTemperature() {
    return sensors->getTempC(sensorAddress);
}

float TemperatureSensor::getFilteredTemperature() {
    return calculateMovingAverage();
}

bool TemperatureSensor::isConnected() {
    // Check if sensor is still responding
    float temp = sensors->getTempC(sensorAddress);
    return (temp != DEVICE_DISCONNECTED_C && temp != -127.0);
}

bool TemperatureSensor::hasError() {
    return !isConnected() || lastTemperature == SENSOR_ERROR_TEMP;
}

void TemperatureSensor::setCalibrationOffset(float offset) {
    temperatureOffset = offset;
    isCalibrated = true;
}

float TemperatureSensor::getCalibrationOffset() {
    return temperatureOffset;
}

void TemperatureSensor::calibrateToReference(float referenceTemp) {
    float currentTemp = getRawTemperature();
    if (validateReading(currentTemp)) {
        temperatureOffset = referenceTemp - currentTemp;
        isCalibrated = true;
        DEBUG_PRINT(F("Calibration offset set to: "));
        DEBUG_PRINTLN(temperatureOffset);
    }
}

int TemperatureSensor::getSensorCount() {
    return sensors->getDeviceCount();
}

void TemperatureSensor::printAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) DEBUG_PRINT("0");
        DEBUG_PRINT(deviceAddress[i], HEX);
    }
}

bool TemperatureSensor::getSensorAddress(DeviceAddress& address, int index) {
    return sensors->getAddress(address, index);
}

void TemperatureSensor::setResolution(uint8_t resolution) {
    sensors->setResolution(sensorAddress, resolution);
}

uint8_t TemperatureSensor::getResolution() {
    return sensors->getResolution(sensorAddress);
}

float TemperatureSensor::getMinTemperature() {
    float minTemp = tempHistory[0];
    for (int i = 1; i < FILTER_SIZE; i++) {
        if (tempHistory[i] < minTemp && tempHistory[i] != 0) {
            minTemp = tempHistory[i];
        }
    }
    return minTemp;
}

float TemperatureSensor::getMaxTemperature() {
    float maxTemp = tempHistory[0];
    for (int i = 1; i < FILTER_SIZE; i++) {
        if (tempHistory[i] > maxTemp) {
            maxTemp = tempHistory[i];
        }
    }
    return maxTemp;
}

float TemperatureSensor::getAverageTemperature() {
    return calculateMovingAverage();
}

void TemperatureSensor::resetStatistics() {
    for (int i = 0; i < FILTER_SIZE; i++) {
        tempHistory[i] = 0.0;
    }
    historyIndex = 0;
    historyFilled = false;
}

void TemperatureSensor::updateMovingAverage(float newTemp) {
    tempHistory[historyIndex] = newTemp;
    historyIndex = (historyIndex + 1) % FILTER_SIZE;
    
    if (historyIndex == 0) {
        historyFilled = true;
    }
}

float TemperatureSensor::calculateMovingAverage() {
    float sum = 0.0;
    int count = historyFilled ? FILTER_SIZE : historyIndex;
    
    if (count == 0) return lastTemperature;
    
    for (int i = 0; i < count; i++) {
        sum += tempHistory[i];
    }
    
    return sum / count;
}

bool TemperatureSensor::validateReading(float temp) {
    // Check if temperature is within valid range
    return (temp > -55.0 && temp < 125.0 && temp != DEVICE_DISCONNECTED_C);
}