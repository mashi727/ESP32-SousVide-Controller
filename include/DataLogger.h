#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include "Config.h"

struct LogEntry {
    unsigned long timestamp;
    float temperature;
    float targetTemp;
    float power;
    unsigned long remainingTime;
};

class DataLogger {
private:
    bool enabled;
    File logFile;
    String currentLogFileName;
    unsigned long logStartTime;
    int entryCount;
    
public:
    DataLogger();
    
    bool begin();
    void logData(float temp, float target, float power, unsigned long remaining);
    void startNewSession();
    void endSession();
    
    String getCurrentLogFileName() { return currentLogFileName; }
    int getEntryCount() { return entryCount; }
    
    bool exportToCSV(String& output);
    void clearAllLogs();
    size_t getUsedSpace();
    size_t getFreeSpace();
    
private:
    String generateFileName();
    bool mountFileSystem();
};

#endif // DATA_LOGGER_H