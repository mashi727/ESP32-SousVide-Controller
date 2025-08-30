#ifndef CONFIG_H
#define CONFIG_H

// Hardware Pin Definitions
#define ONE_WIRE_BUS        4      // DS18B20 temperature sensor
#define OLED_SDA            21     // I2C SDA for OLED display
#define OLED_SCL            22     // I2C SCL for OLED display
#define ENCODER_PIN_A       32     // Rotary encoder A
#define ENCODER_PIN_B       33     // Rotary encoder B
#define ENCODER_BUTTON_PIN  25     // Rotary encoder push button
#define SSR_PIN             26     // Solid State Relay control
#define BUZZER_PIN          27     // Optional buzzer for alerts

// Display Configuration
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define OLED_ADDRESS        0x3C
#define DISPLAY_UPDATE_INTERVAL 250  // ms

// Temperature Sensor Configuration
#define TEMP_RESOLUTION     12     // DS18B20 resolution (9-12 bits)
#define TEMP_READ_INTERVAL  750    // ms
#define TEMP_SENSOR_COUNT   1      // Number of temperature sensors

// PID Control Parameters
#define DEFAULT_KP          2.0
#define DEFAULT_KI          0.5
#define DEFAULT_KD          1.0
#define PID_WINDOW_SIZE     5000   // ms (5 seconds)
#define PID_SAMPLE_TIME     1000   // ms

// Cooking Parameters
#define DEFAULT_TARGET_TEMP 56.0   // °C
#define MIN_TEMP            20.0   // °C
#define MAX_TEMP            95.0   // °C
#define TEMP_STEP           0.5    // °C
#define DEFAULT_COOKING_TIME 3600  // seconds (1 hour)
#define MIN_COOKING_TIME    60     // seconds
#define MAX_COOKING_TIME    172800 // seconds (48 hours)
#define TIME_STEP           60     // seconds

// Safety Features
#define MAX_TEMP_LIMIT      100.0  // °C - absolute maximum temperature
#define TEMP_ALARM_THRESHOLD 5.0   // °C - deviation from target to trigger alarm
#define SENSOR_ERROR_TEMP   -127.0 // Error value from DS18B20
#define WATCHDOG_TIMEOUT    10000  // ms

// Data Logging
#define ENABLE_DATA_LOGGING true
#define DATA_LOG_INTERVAL   10000  // ms (10 seconds)
#define MAX_LOG_ENTRIES     1000
#define LOG_TO_SPIFFS       true
#define LOG_TO_SD_CARD      false

// WiFi Configuration
#define ENABLE_WIFI         true
#define WIFI_SSID           "YourWiFiSSID"
#define WIFI_PASSWORD       "YourWiFiPassword"
#define MDNS_HOSTNAME       "sousvide"
#define WEB_SERVER_PORT     80
#define WEBSOCKET_PORT      81
#define AP_MODE_ENABLED     false  // Enable Access Point mode if WiFi fails
#define AP_SSID             "SousVide-AP"
#define AP_PASSWORD         "12345678"

// MQTT Configuration (optional)
#define ENABLE_MQTT         false
#define MQTT_SERVER         "192.168.1.100"
#define MQTT_PORT           1883
#define MQTT_USER           ""
#define MQTT_PASSWORD       ""
#define MQTT_CLIENT_ID      "esp32_sousvide"
#define MQTT_TOPIC_TEMP     "sousvide/temperature"
#define MQTT_TOPIC_SETPOINT "sousvide/setpoint"
#define MQTT_TOPIC_STATUS   "sousvide/status"
#define MQTT_TOPIC_COMMAND  "sousvide/command"

// Encoder Configuration
#define ENCODER_STEPS_PER_NOTCH 4
#define BUTTON_DEBOUNCE_TIME    50   // ms
#define BUTTON_LONG_PRESS_TIME  1000 // ms

// State Machine States
enum SystemState {
    STATE_IDLE,
    STATE_SETUP_TEMP,
    STATE_SETUP_TIME,
    STATE_PREHEAT,
    STATE_COOKING,
    STATE_FINISHED,
    STATE_ERROR,
    STATE_CALIBRATION,
    STATE_WIFI_CONFIG
};

// Error Codes
enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_SENSOR_DISCONNECTED,
    ERROR_OVERTEMPERATURE,
    ERROR_UNDERTEMPERATURE,
    ERROR_PID_FAILURE,
    ERROR_SSR_FAILURE,
    ERROR_WIFI_CONNECTION,
    ERROR_MEMORY_FULL
};

// Cooking Parameters Structure
struct CookingParameters {
    float targetTemperature;
    unsigned long cookingTime;  // in seconds
    float pidKp;
    float pidKi;
    float pidKd;
    bool preHeatEnabled;
    bool alarmEnabled;
};

// System Configuration Structure
struct SystemConfig {
    bool wifiEnabled;
    bool mqttEnabled;
    bool loggingEnabled;
    bool soundEnabled;
    char wifiSSID[32];
    char wifiPassword[64];
    char mqttServer[64];
    int mqttPort;
};

// Debug Configuration
#define DEBUG_SERIAL        true
#define DEBUG_LEVEL         2      // 0=None, 1=Error, 2=Info, 3=Verbose

#if DEBUG_SERIAL
    #define DEBUG_PRINT(x)    Serial.print(x)
    #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H