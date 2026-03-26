// Sim7600Manager.h
#ifndef SIM7600_H
#define SIM7600_H

#include <Arduino.h>
#include "debug.h"

// --- Delay Configuration ---
// Define SIM7600_USE_FREERTOS before including this header to use vTaskDelay.
// Default: uses Arduino delay()
#ifdef SIM7600_USE_FREERTOS
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
  #define SIM7600_DELAY(ms)  vTaskDelay((ms) / portTICK_PERIOD_MS)
  #define SIM7600_YIELD()    vTaskDelay(1)
#else
  #define SIM7600_DELAY(ms)  delay(ms)
  #define SIM7600_YIELD()    yield()
#endif

#include "SIM7600_MQTT.h"
#include "SIM7600_HTTP.h"
#include "SIM7600_NTP.h"

class Sim7600MQTT;
class Sim7600HTTP;
class Sim7600NTP;

class Sim7600Manager
{
public:
    // Constructor: Takes the serial port the modem is connected to
    Sim7600Manager(HardwareSerial &serial);
    Sim7600MQTT mqtt;
    Sim7600HTTP http;
    Sim7600NTP ntp;

    // --- Core Functions ---
    bool begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin);
    bool initializeModem();
    String getSimCCID();
    bool connectGPRS(const char *apn);
    bool isGPRSConnected();\
    int signalQuality();
    void fullDisconnect();
    void clearSerialBuffer();
    bool sendATCommand(const char *cmd, const char *expected_response, unsigned long timeout);
        bool sendATCommand(const char *cmd, const char *expected_response, unsigned long timeout, String &resp);
    // ... (your sendATCommandWithData would be overloaded similarly)
    bool sendATCommandWithData(const char *cmd, const char *data, const char *expected_response, unsigned long timeout);
    String readResponse(const char* endMarker, unsigned long timeout = 5000);
    int extractLastCode(const String &response);
    bool parseHttpActionResponse(const String &response, int &statusCode, int &dataLength);
    String getLastResponse(unsigned long timeout = 2000);
    
int signalQualityImmediate()
    {
        lastSignalCheck = 0; // Reset timer to force update
        return signalQuality();
    }



 

private:
    // ...

// // --- HTTP Functions ---
// String httpsGET(const char *url);

// // --- MQTT Functions ---
// bool connectMQTT(const char *broker, int port, const char *clientId, const char *user, const char *pass, int clientIndex = 0);
// bool subscribe(const char *topic, int qos = 0);
// bool publish(const char *topic, const char *message, int qos = 0);
// bool isMqttConnected();
// void maintain(); // Call this in your loop() to process incoming data
// HardwareSerial &getSerial() { return _serial; }
HardwareSerial & _serial;
friend class Sim7600MQTT;
friend class Sim7600HTTP;
friend class Sim7600NTP;

   unsigned long lastSignalCheck = 0;  // ✅ Add this
    int cachedSignal = -1;    


String printError(const String &response, int &outCode);
String decodeMQTTError(const String &response, int &outCode);
String decodeHTTPError(const String &response, int &outCode);
String decodeTCPError(const String &response, int &outCode);
String decodeSSLError(const String &response, int &outCode);
String decodeGeneralError(const String &response, int &outCode);

// // --- Low-level AT Command Helpers ---
// );
}
;

#endif