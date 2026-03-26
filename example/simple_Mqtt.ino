#include <SIM7600.h>

Sim7600Manager modem(Serial2);

// MQTT Settings
const char* broker = "broker.hivemq.com";
int port = 1883;
const char* topic_sub = "sim7600/test/rx";
const char* topic_pub = "sim7600/test/tx";

// Callback function for incoming messages
void onMessageReceived(const String& topic, const String& payload) {
    Serial.println("\n--- New Message Received ---");
    Serial.print("Topic: "); Serial.println(topic);
    Serial.print("Payload: "); Serial.println(payload);
    Serial.println("---------------------------\n");
}

void setup() {
    Serial.begin(115200);
    modem.begin(115200, SERIAL_8N1, 16, 17);
    modem.connectGPRS("your_apn");

    // 1. Set the callback function
    modem.mqtt.setCallback(onMessageReceived);

    // 2. Connect to MQTT Broker
    Serial.println("Connecting to MQTT...");
    if (modem.mqtt.connect(broker, port, "ESP32_SIM7600_Client")) {
        Serial.println("MQTT Connected!");
        
        // 3. Subscribe to a topic
        modem.mqtt.subscribe(topic_sub, 0);
    }
}

void loop() {
    // Required: Call loop() to process incoming MQTT data
    modem.mqtt.loop();

    // Publish data every 15 seconds
    static unsigned long lastPub = 0;
    if (millis() - lastPub > 15000) {
        lastPub = millis();
        String message = "Hello from SIM7600! Signal: " + String(modem.signalQuality());
        modem.mqtt.publish(topic_pub, message.c_str());
        Serial.println("Published heartbeat.");
    }
}