#include <SIM7600.h>

// Define pins for ESP32 (Change if using different board)
#define MODEM_RX 16
#define MODEM_TX 17

// Use Serial2 for the modem
Sim7600Manager modem(Serial2);

const char* apn = "your_apn_here"; // e.g., "internet" or "hologram"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- SIM7600 Basic Setup ---");

    // 1. Initialize Hardware (Baud, Config, RX, TX)
    if (!modem.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX)) {
        Serial.println("Failed to communicate with modem!");
        while(1); // Halt
    }

    // 2. Get SIM Info
    String ccid = modem.getSimCCID();
    Serial.println("SIM CCID: " + ccid);

    // 3. Connect to GPRS
    Serial.println("Connecting to GPRS...");
    if (modem.connectGPRS(apn)) {
        Serial.println("GPRS Connected Successfully!");
    } else {
        Serial.println("GPRS Connection Failed.");
    }
}

void loop() {
    // Check signal quality every 10 seconds (handled internally by the library)
    int signal = modem.signalQuality();
    Serial.printf("Signal Strength: %d/31\n", signal);

    if (modem.isGPRSConnected()) {
        Serial.println("Status: Online");
    } else {
        Serial.println("Status: Offline - Attempting reconnect...");
        modem.connectGPRS(apn);
    }

    delay(10000);
}