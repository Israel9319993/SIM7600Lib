#include <SIM7600.h>

Sim7600Manager modem(Serial2);

void setup() {
    Serial.begin(115200);
    modem.begin(115200, SERIAL_8N1, 16, 17);
    modem.connectGPRS("your_apn");

    // 1. Fetch NTP Time
    String timeStr;
    if (modem.ntp.fetchNTPTime("pool.ntp.org", 0, timeStr)) {
        Serial.println("Current Time: " + timeStr);
    }

    // 2. Simple HTTPS GET request
    Serial.println("Fetching JSON data...");
    String json = modem.http.httpsGET("https://jsonplaceholder.typicode.com/posts/1");
    if (json != "") {
        Serial.println("Response: " + json);
    }

    // 3. OTA Update Trigger (Example)
    // Serial.println("Starting OTA Update...");
    // OTAState ota;
    // if (modem.http.performOTA("http://your-server.com/firmware.bin", ota)) {
    //     Serial.println("Update success! Rebooting...");
    //     ESP.restart();
    // }
}

void loop() {
    // Nothing here
}