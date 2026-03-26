#ifndef SIM7600_MQTT_H
#define SIM7600_MQTT_H

#include <Arduino.h>

// Forward declaration to avoid circular dependency
class Sim7600Manager;




class Sim7600MQTT {
public:
    // --- Constructor ---
    explicit Sim7600MQTT(Sim7600Manager& manager);

    // --- MQTT Core Functions ---
    bool connect(const char* broker, int port, const char* clientId,
                 const char* user = nullptr, const char* pass = nullptr,
                 int clientIndex = 0, const char* willTopic = nullptr,
                 uint8_t willQos = 0, const char* willMessage = nullptr,
                 bool cleanSession = true);

    bool connectSecure(const char* broker, int port, const char* clientId,
                       const char* user = nullptr, const char* pass = nullptr,
                       int clientIndex = 0, const char* willTopic = nullptr,
                       uint8_t willQos = 0, const char* willMessage = nullptr,
                       bool cleanSession = true, const char* ca_cert_path = nullptr,
                       int ssl_ctx_index = 0);

    bool disconnect();
    bool subscribe(const char* topic, int qos = 0);
    bool publish(const char* topic, const char* message, bool retained = false, int qos = 0);

    bool isMqttConnected();
    bool isConnected();

    // --- Processing Incoming Data ---
    void loop(); 

    // Set callback for incoming messages
    void setCallback(void (*cb)(const String& topic, const String& payload));
    

private:
    Sim7600Manager& _manager;      // Reference to the parent manager
    bool _is_connected = false;    // Connection state
    int _client_index = 0;         // Client ID index

    // Internal message parsing
    void handleIncomingData(const char* line);
    
    // REMOVE static - make them instance variables
    String topicBuffer;
    String payloadBuffer;
    bool readingTopic;
    bool readingPayload;
    
    //  REMOVE static - make it instance variable
    void (*userCallback)(const String&, const String&);
};

#endif // SIM7600_MQTT_H