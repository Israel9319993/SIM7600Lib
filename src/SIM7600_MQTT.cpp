// Sim7600MQTT.cpp
#include "SIM7600_MQTT.h"
#include "SIM7600.h" // Include the main manager header
                     // #include "SIM7600_ERROR.h"

enum MQTTState
{
    MQTT_IDLE,
    MQTT_PUBLISHING,
    MQTT_SUBSCRIBING,
    MQTT_WAIT_RESPONSE
};
MQTTState mqttState = MQTT_IDLE;

Sim7600MQTT::Sim7600MQTT(Sim7600Manager &manager) : _manager(manager),
                                                    _is_connected(false),
                                                    _client_index(0),
                                                    readingTopic(false),
                                                    readingPayload(false),
                                                    userCallback(nullptr)
{
    topicBuffer = "";
    payloadBuffer = "";
}
bool Sim7600MQTT::connect(const char *broker, int port, const char *clientId, const char *user, const char *pass, int clientIndex, const char *willTopic, uint8_t willQos, const char *willMessage, bool cleanSession)
{
    _client_index = clientIndex;
    char cmd_buffer[256];
    char rsp_buffer[256];
    int ssl = 0;
    int session = cleanSession ? 1 : 0;

    disconnect(); // Ensure any existing connection is closed
    SIM7600_DELAY(3000);
    _manager.sendATCommand("AT+CMQTTSTART", "+CMQTTSTART: 0", 30000);

    // ✅ Configure MQTT
    Debug_Println("⚙️ Configuring MQTT receive mode (secure)...");

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTCFG=\"checkUTF8\",%d,0", _client_index);
    _manager.sendATCommand(cmd_buffer, "OK", 5000);
    SIM7600_DELAY(200);

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTCFG=\"optimeout\",%d,120", _client_index);
    _manager.sendATCommand(cmd_buffer, "OK", 5000);
    SIM7600_DELAY(200);

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTACCQ=%d,\"%s\",%d", _client_index, clientId, ssl);
    _manager.sendATCommand(cmd_buffer, "OK", 5000);

    if (willTopic != NULL && willMessage != NULL)
    {
        // Set Last Will and Testament
        snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTWILLTOPIC=%d,%d", _client_index, strlen(willTopic));
        if (!_manager.sendATCommandWithData(cmd_buffer, willTopic, "OK", 5000))
            return false;

        snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTWILLMSG=%d,%d,%d", _client_index, strlen(willMessage), willQos);
        if (!_manager.sendATCommandWithData(cmd_buffer, willMessage, "OK", 5000))
            return false;
    }
    int response;
    snprintf(cmd_buffer, sizeof(cmd_buffer),
             "AT+CMQTTCONNECT=%d,\"tcp://%s:%d\",60,%d,\"%s\",\"%s\"",
             _client_index, broker, port, session, user, pass);

    snprintf(rsp_buffer, sizeof(rsp_buffer),
             "+CMQTTCONNECT: %d,0", _client_index);
    if (!_manager.sendATCommand(cmd_buffer, rsp_buffer, 30000))
    {
        // Debug_Printf("ERROR: MQTT Connection Failed! Code %d: %s\n", response, getSim7600ErrorString(SIM7600_ERR_MQTT, response));
        return false;
    }

    _is_connected = true;
    return true;
}

bool Sim7600MQTT::connectSecure(const char *broker, int port, const char *clientId, const char *user, const char *pass, int clientIndex, const char *willTopic, uint8_t willQos, const char *willMessage, bool cleanSession, const char *ca_cert_path, int ssl_ctx_index)
{
    clientIndex = clientIndex;
    char cmd_buffer[256];
    char rsp_buffer[256];
    int ssl = 1;
    int session = cleanSession ? 1 : 0;

    disconnect(); // Ensure any existing connection is closed
                  // Give some time for the disconnect to process
    _manager.sendATCommand("AT+CMQTTSTART", "+CMQTTSTART: 0", 30000);
    SIM7600_DELAY(1000);

    // ✅ Configure MQTT
    Debug_Println("⚙️ Configuring MQTT receive mode (secure)...");

    // snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTCFG=\"checkUTF8\",%d,0", _client_index);
    // _manager.sendATCommand(cmd_buffer, "OK", 5000);
    // delay(200);

    // snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTCFG=\"optimeout\",%d,120", _client_index);
    // _manager.sendATCommand(cmd_buffer, "OK", 5000);
    // delay(200);

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTACCQ=%d,\"%s\",%d", _client_index, clientId, ssl);
    _manager.sendATCommand(cmd_buffer, "OK", 5000);

    if (willTopic != NULL && willMessage != NULL)
    {
        // Set Last Will and Testament
        snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTWILLTOPIC=%d,%d", _client_index, strlen(willTopic));
        _manager.sendATCommandWithData(cmd_buffer, willTopic, "OK", 5000);
            // return false;

        snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTWILLMSG=%d,%d,%d", _client_index, strlen(willMessage), willQos);
        _manager.sendATCommandWithData(cmd_buffer, willMessage, "OK", 5000);
            // return false;
    }
    //  response;
    snprintf(cmd_buffer, sizeof(cmd_buffer),
             "AT+CMQTTCONNECT=%d,\"tcp://%s:%d\",60,%d,\"%s\",\"%s\"",
             _client_index, broker, port, session, user, pass);

    snprintf(rsp_buffer, sizeof(rsp_buffer),
             "+CMQTTCONNECT: %d,0", _client_index);

    if (!_manager.sendATCommand(cmd_buffer, rsp_buffer, 30000))
    {
        return false;
    }

    SIM7600_DELAY(2000);
    _is_connected = true;
    return true;
}

// (You would add the connectSecure implementation here too)

bool Sim7600MQTT::subscribe(const char *topic, int qos)
{
    if (!_is_connected)
        return false;

    char cmd_buffer[200];
    char rsp_buffer[100];
    
    // Step 1: Set subscription topic (sends data after '>' prompt)
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTSUBTOPIC=%d,%d,%d", _client_index, strlen(topic), qos);
    if (!_manager.sendATCommandWithData(cmd_buffer, topic, "OK", 5000)) {
        Debug_Println("❌ Failed to set subscription topic: " + String(topic));
        return false;
    }
    SIM7600_DELAY(300);
    
    // Step 2: Execute subscription and wait for the URC "+CMQTTSUB: 0,0" (async response)
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTSUB=%d", _client_index);
    snprintf(rsp_buffer, sizeof(rsp_buffer), "+CMQTTSUB: %d,0", _client_index);
    if (_manager.sendATCommand(cmd_buffer, rsp_buffer, 10000)) {
        Debug_Println("✅ Subscribed to: " + String(topic));
        return true;
    }
    
    Debug_Println("❌ Failed to subscribe to: " + String(topic));
    return false;
}

bool Sim7600MQTT::publish(const char *topic, const char *message, bool retained, int qos)
{
    // Check if connected
    if (!_is_connected)
        return false;
    
    // Wait for MQTT to become idle (with timeout)
    unsigned long startWait = millis();
    while (mqttState != MQTT_IDLE && (millis() - startWait) < 10000) {
        SIM7600_DELAY(100);
    }
    
    // If still not idle after timeout, force reset
    if (mqttState != MQTT_IDLE) {
        Debug_Println("⚠️ MQTT state stuck, forcing reset...");
        mqttState = MQTT_IDLE;
        SIM7600_DELAY(500);
    }

    mqttState = MQTT_PUBLISHING;
    char cmd_buffer[256];

    int retainedFlag = retained ? 1 : 0;

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTTOPIC=%d,%d", _client_index, strlen(topic));
    if (!_manager.sendATCommandWithData(cmd_buffer, topic, "OK", 5000)) {
        mqttState = MQTT_IDLE;
        return false;
    }

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTPAYLOAD=%d,%d", _client_index, strlen(message));
    if (!_manager.sendATCommandWithData(cmd_buffer, message, "OK", 5000)) {
        mqttState = MQTT_IDLE;
        return false;
    }

    // BUG FIX: was "AT+CMQTTPUB=%d,%d,60" — missing retained flag placeholder
    // Per AT manual: AT+CMQTTPUB=<client_index>,<qos>,<pub_timeout>[,<retained>[,<dup>]]
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTPUB=%d,%d,60,%d", _client_index, qos, retainedFlag);
    bool result = _manager.sendATCommand(cmd_buffer, "OK", 20000);
    
    mqttState = MQTT_IDLE;
    SIM7600_DELAY(200); // Small delay between publishes
    return result;
}

bool Sim7600MQTT::isMqttConnected()
{
    return _is_connected;
}

bool Sim7600MQTT::isConnected()
{
    // This function now performs a live check with the modem.
    char cmd_buffer[50];
    char rsp_buffer[50];

    // Prepare the command and the expected success response string
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTCONNECT?");
    snprintf(rsp_buffer, sizeof(rsp_buffer), "+CMQTTCONNECT: %d,0", _client_index);

    // Send the command. We don't need a long timeout for a simple query.
    // If the response contains "+CMQTTCONNECT: 0" (for client 0), it means
    // the modem considers that client to be actively connected.
    if (_manager.sendATCommand(cmd_buffer, rsp_buffer, 3000))
    {
        // If the command succeeded and we found our client in the response,
        // update our internal state and return true.
        _is_connected = true;
        return true;
    }

    // If the command fails or the expected response is not found,
    // it means the client is not connected.
    _is_connected = false;
    return false;
}

// Your disconnect function should use this check
bool Sim7600MQTT::disconnect()
{
    // Now we use our robust isConnected() check first.
    // if (!isConnected()) {
    //     Debug_Println("INFO: MQTT already disconnected.");
    //     return true;
    // }

    // The rest of your corrected disconnect logic follows...
    Debug_Println("\n--- Disconnecting MQTT Client ---");
    char cmd_buffer[50];

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTDISC=%d,120", _client_index);
    _manager.sendATCommand(cmd_buffer, "OK", 5000);
    SIM7600_DELAY(2000); // Give some time for the disconnect to process
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CMQTTREL=%d", _client_index);
    _manager.sendATCommand(cmd_buffer, "OK", 5000);
    SIM7600_DELAY(2000);
    _manager.sendATCommand("AT+CMQTTSTOP", "OK", 5000);
    SIM7600_DELAY(2000);
    _is_connected = false; // Final state update
    Debug_Println("MQTT disconnected successfully.");
    return true;
}

void Sim7600MQTT::setCallback(void (*cb)(const String &, const String &))
{
    userCallback = cb;
    Debug_Println("✅ Callback registered!");
}

void Sim7600MQTT::handleIncomingData(const char *line)
{
    Debug_Print("📥 ");
    Debug_Println(line);

    if (strncmp(line, "+CMQTTRXSTART", 13) == 0)
    {
        Debug_Println("🔵 START");
        topicBuffer = "";
        payloadBuffer = "";
        readingTopic = false;
        readingPayload = false;
         mqttState = MQTT_WAIT_RESPONSE;
    }
    else if (strncmp(line, "+CMQTTRXTOPIC", 13) == 0)
    {
        Debug_Println("🔵 TOPIC MODE");
        readingTopic = true;
        readingPayload = false;
        topicBuffer = "";
    }
    else if (strncmp(line, "+CMQTTRXPAYLOAD", 15) == 0)
    {
        Debug_Println("🔵 PAYLOAD MODE");
        readingTopic = false;
        readingPayload = true;
        payloadBuffer = "";
    }
    else if (strncmp(line, "+CMQTTRXEND", 11) == 0)
    {
        Debug_Println("🔵 END");
        Debug_Print("✉️ Topic: [");
        Debug_Print(topicBuffer);
        Debug_Println("]");
        Debug_Print("📄 Payload: [");
        Debug_Print(payloadBuffer);
        Debug_Println("]");

        readingTopic = false;
        readingPayload = false;
         mqttState = MQTT_IDLE;

        if (userCallback)
        {
            if (topicBuffer.length() > 0)
            {
                Debug_Println("✅ CALLING CALLBACK NOW!");
                userCallback(topicBuffer, payloadBuffer);
            }
            else
            {
                Debug_Println("❌ Topic empty!");
            }
        }
        else
        {
            Debug_Println("❌ NO CALLBACK SET!");
        }

        topicBuffer = "";
        payloadBuffer = "";
    }
    else
    {
        if (strlen(line) == 0)
            return;
        if (strncmp(line, "AT", 2) == 0)
            return;
        if (strncmp(line, ">>>", 3) == 0)
            return;
        if (strncmp(line, "OK", 2) == 0)
            return;

        if (readingTopic)
        {
            Debug_Print("  ➕ Topic += ");
            Debug_Println(line);
            topicBuffer += line;
        }
        else if (readingPayload)
        {
            Debug_Print("  ➕ Payload += ");
            Debug_Println(line);
            if (payloadBuffer.length() > 0)
            {
                payloadBuffer += "\n";
            }
            payloadBuffer += line;
        }
    }
}

void Sim7600MQTT::loop()
{
    // ✅ Use while loop to process ALL data
    while (_manager._serial.available())
    {
        Debug_Print("🔍 Checking for incoming data...  com\n");
        String line = _manager._serial.readStringUntil('\n');
        line.trim();
        if (line.length() > 0)
        {
            handleIncomingData(line.c_str());
        }
    }
}