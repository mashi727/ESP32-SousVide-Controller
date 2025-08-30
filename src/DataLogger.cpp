#include "../include/DataLogger.h"

DataLogger::DataLogger() {
    enabled = false;
    currentLogFileName = "";
    logStartTime = 0;
    entryCount = 0;
}

bool DataLogger::begin() {
    if (!mountFileSystem()) {
        return false;
    }
    
    enabled = true;
    startNewSession();
    return true;
}

void DataLogger::logData(float temp, float target, float power, unsigned long remaining) {
    if (!enabled) return;
    
    LogEntry entry;
    entry.timestamp = millis() - logStartTime;
    entry.temperature = temp;
    entry.targetTemp = target;
    entry.power = power;
    entry.remainingTime = remaining;
    
    if (logFile) {
        logFile.print(entry.timestamp / 1000);
        logFile.print(",");
        logFile.print(entry.temperature, 2);
        logFile.print(",");
        logFile.print(entry.targetTemp, 2);
        logFile.print(",");
        logFile.print(entry.power, 1);
        logFile.print(",");
        logFile.println(entry.remainingTime);
        logFile.flush();
        
        entryCount++;
        
        if (entryCount >= MAX_LOG_ENTRIES) {
            endSession();
            startNewSession();
        }
    }
}

void DataLogger::startNewSession() {
    currentLogFileName = generateFileName();
    logFile = SPIFFS.open(currentLogFileName, FILE_WRITE);
    
    if (logFile) {
        logFile.println("Time(s),Temperature(C),Target(C),Power(%),Remaining(s)");
        logStartTime = millis();
        entryCount = 0;
        DEBUG_PRINT(F("Started new log: "));
        DEBUG_PRINTLN(currentLogFileName);
    }
}

void DataLogger::endSession() {
    if (logFile) {
        logFile.close();
        DEBUG_PRINTLN(F("Log session ended"));
    }
}

String DataLogger::generateFileName() {
    return "/log_" + String(millis()) + ".csv";
}

bool DataLogger::mountFileSystem() {
    if (!SPIFFS.begin(true)) {
        DEBUG_PRINTLN(F("SPIFFS mount failed"));
        return false;
    }
    
    DEBUG_PRINTLN(F("SPIFFS mounted successfully"));
    return true;
}

bool DataLogger::exportToCSV(String& output) {
    if (!currentLogFileName.isEmpty()) {
        File file = SPIFFS.open(currentLogFileName, FILE_READ);
        if (file) {
            output = file.readString();
            file.close();
            return true;
        }
    }
    return false;
}

void DataLogger::clearAllLogs() {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    
    while (file) {
        String fileName = file.name();
        file.close();
        
        if (fileName.startsWith("/log_")) {
            SPIFFS.remove(fileName);
        }
        
        file = root.openNextFile();
    }
    
    root.close();
}

size_t DataLogger::getUsedSpace() {
    return SPIFFS.usedBytes();
}

size_t DataLogger::getFreeSpace() {
    return SPIFFS.totalBytes() - SPIFFS.usedBytes();
}