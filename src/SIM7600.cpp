// Sim7600Manager.cpp
#include "SIM7600.h"
#include "SIM7600_ERROR.h" // include your universal error header

Sim7600Manager::Sim7600Manager(HardwareSerial &serial) : _serial(serial),
                                                         mqtt(*this),
                                                         http(*this),
                                                         ntp(*this)
{
    // Constructor body
}

bool Sim7600Manager::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin)
{
    _serial.begin(baud, config, rxPin, txPin);
       _serial.setRxBufferSize(4096); 
    SIM7600_DELAY(1000);
    return initializeModem();
}

// --- Core Functions Implementation ---


bool Sim7600Manager::initializeModem()
{
    Debug_Println("\n--- Initializing SIM7600 ---");
    for (int i = 0; i < 3; i++)
    {
        if (sendATCommand("AT", "OK", 1000))
            break;
        SIM7600_DELAY(1000);
    }
    if (!sendATCommand("ATE0", "OK", 1000))
        return false;
    if (!sendATCommand("AT+CPIN?", "READY", 5000))
    {
        Debug_Println("SIM card not ready!");
        return false;
    }
    sendATCommand("AT+CSQ", "OK", 2000);
    Debug_Println("Waiting for network registration...");
    for (int i = 0; i < 20; i++)
    {
        if (sendATCommand("AT+CREG?", "+CREG: 0,1", 1000) ||
            sendATCommand("AT+CREG?", "+CREG: 0,5", 1000))
        {
            Debug_Println("Network registration successful!");
            return true;
        }
        SIM7600_DELAY(1000);
    }
    Debug_Println("Network registration timed out.");
    return false;
}

String Sim7600Manager::getSimCCID()
{
    Debug_Println("\n--- Retrieving SIM CCID ---");
    String response;
    if (sendATCommand("AT+CCID", "OK", 2000, response))
    {
        int start = response.indexOf("+CCID: ");
        if (start != -1)
        {
            start += 7; // Length of "+CCID: "
            int end = response.indexOf("\r", start);
            if (end != -1)
            {
                String ccid = response.substring(start, end);
                Debug_Print("SIM CCID: ");
                Debug_Println(ccid);
                return ccid;
            }
        }
    }
    Debug_Println("Failed to retrieve SIM CCID.");
    return "";
}

bool Sim7600Manager::connectGPRS(const char *apn)
{
     Debug_Println("Waiting for network registration...");
    // for (int i = 0; i < 20; i++)
    // {
    //     if (!sendATCommand("AT+CREG?", "+CREG: 0,1", 1000) ||
    //         !sendATCommand("AT+CREG?", "+CREG: 0,5", 1000))
    //     {
    //         Debug_Println("Network registration not successful!");
    //         return false;
    //     }
    //     delay(1000);
    // }

    // Debug_Println("Network registration timed out.");
    // Debug_Println("\n--- Starting GPRS Connection ---");
    char cmd_buffer[128];
    // snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CGDCONT=1,\"IP\",\"%s\"", apn);
    // if (!sendATCommand(cmd_buffer, "OK", 5000))
    //     return false;
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CGSOCKCONT=1,\"IP\",\"%s\"", apn);
    if (!sendATCommand(cmd_buffer, "OK", 5000))
        return false;
    if (!sendATCommand("AT+CGACT=1,1", "OK", 10000))
        return false;
    sendATCommand("AT+CGPADDR", "OK", 1000);
    Debug_Println("GPRS connection established.");
    return true;
}

bool Sim7600Manager::isGPRSConnected()
{
    String response;
    if (sendATCommand("AT+CGACT?", "OK", 2000, response))
    {
        int index = response.indexOf("+CGACT: 1,1");
        if (index != -1)
        {
            return true;
        }
    }
    return false;
}


int Sim7600Manager::signalQuality()
{
    unsigned long currentTime = millis();
    
    // ✅ Only update every 10 seconds
    if (currentTime - lastSignalCheck >= 20000)
    {
        lastSignalCheck = currentTime;
        
        String response;
        if (sendATCommand("AT+CSQ", "OK", 2000, response))
        {
            // Response format: +CSQ: <rssi>,<ber>
            // Example: +CSQ: 25,0
            int start = response.indexOf("+CSQ: ");
            if (start != -1)
            {
                start += 6; // Skip "+CSQ: "
                int comma = response.indexOf(",", start);
                if (comma != -1)
                {
                    String rssiStr = response.substring(start, comma);
                    rssiStr.trim(); // Remove whitespace
                    int rssi = rssiStr.toInt();
                    
                    // Validate RSSI range (0-31 = valid, 99 = not known/detectable)
                    if (rssi >= 0 && rssi <= 31)
                    {
                        cachedSignal = rssi;
                        Debug_Printf("📶 Signal: %d/31\n", rssi);
                        return cachedSignal;
                    }
                    else if (rssi == 99)
                    {
                        Debug_Println("⚠️ Signal not detectable");
                        cachedSignal = 0; // No signal
                        return cachedSignal;
                    }
                }
            }
            
            Debug_Println("❌ Failed to parse signal quality");
            cachedSignal = -1;
        }
        else
        {
            Debug_Println("❌ Failed to get signal quality");
            cachedSignal = -1;
        }
    }
    
    // ✅ Return cached value if within 10 second window
    return cachedSignal;
}
void Sim7600Manager::fullDisconnect()
{
    Debug_Println("\n--- Performing Full Disconnection ---");
    sendATCommand("AT+CMQTTDISC=0,120", "OK", 5000);
    SIM7600_DELAY(1000);
    // BUG FIX: was missing AT+CMQTTREL — causes error 19 (Client already in use) on next connect
    sendATCommand("AT+CMQTTREL=0", "OK", 5000);
    SIM7600_DELAY(1000);
    sendATCommand("AT+CMQTTSTOP", "OK", 5000);
    SIM7600_DELAY(1000);
    sendATCommand("AT+NETCLOSE", "OK", 5000);
    sendATCommand("AT+CGACT=0,1", "OK", 5000);
}


// --- Private Helper Functions Implementation ---

void Sim7600Manager::clearSerialBuffer()
{
    SIM7600_DELAY(100);
    while (_serial.available())
    {
        _serial.read();
    }
}


bool Sim7600Manager::sendATCommand(const char *cmd, const char *expected_response, unsigned long timeout)
{
    Debug_Printf(">>> %s\n", cmd);
    clearSerialBuffer();
    _serial.println(cmd);

    unsigned long start_time = millis();
    unsigned long last_char_time = millis();
    String response = "";
    const unsigned long SETTLE_TIME = 100; // Wait 100ms after last character

    while (millis() - start_time < timeout)
    {
        if (_serial.available())
        {
            char c = _serial.read();
            response += c;
            last_char_time = millis();
        }
        else
        {
            SIM7600_YIELD(); // Yield to scheduler when no data available
        }

        // ✅ Check if response has settled (no new data for SETTLE_TIME)
        if (response.length() > 0 && (millis() - last_char_time) >= SETTLE_TIME)
        {
            Debug_Print("<<< ");
            Debug_Println(response);

            // Check for OK (success)
            if (response.indexOf("OK") != -1)
            {
                // Verify expected response is present (if specified)
                if (expected_response == nullptr || 
                    strlen(expected_response) == 0 || 
                    response.indexOf(expected_response) != -1)
                {
                    return true;
                }
                else
                {
                    // OK received but expected URC hasn't arrived yet — keep waiting
                    // (async commands like AT+CMQTTCONNECT return OK first, then URC later)
                    last_char_time = millis();
                }
            }

            // ❌ Handle ERROR responses
            if (response.indexOf("ERROR") != -1 || 
                response.indexOf("+CME ERROR") != -1 || 
                response.indexOf("+CMS ERROR") != -1)
            {
                int res;
                printError(response, res);
                return false;
            }

            // Got complete response but no OK/ERROR - keep waiting
            // Reset settle timer
            last_char_time = millis();
        }
    }

    // ⏱ Timeout handling
    Debug_Print("<<< ⏱ TIMEOUT waiting for: ");
    Debug_Println(expected_response);
    Debug_Println(response);
    Debug_Println("Error: Timeout or no response from module");
    return false;
}

bool Sim7600Manager::sendATCommand(const char *cmd,
                                   const char *expected_response,
                                   unsigned long timeout,
                                   String &resp)
{
    clearSerialBuffer();   // moved here

    Debug_Printf(">>> %s\n", cmd);
    _serial.println(cmd);

    unsigned long start_time = millis();
    unsigned long last_char_time = millis();
    String response = "";
    const unsigned long SETTLE_TIME = 100;

    while (millis() - start_time < timeout)
    {
        if (_serial.available())
        {
            char c = _serial.read();
            response += c;
            last_char_time = millis();
        }
        else
        {
            SIM7600_YIELD();
        }

        if (response.length() > 0 &&
            (millis() - last_char_time) >= SETTLE_TIME)
        {
            Debug_Print("<<< ");
            Debug_Println(response);
            resp = response;

            if (response.indexOf("OK") != -1)
            {
                if (expected_response == nullptr ||
                    strlen(expected_response) == 0 ||
                    response.indexOf(expected_response) != -1)
                {
                    return true;
                }
                else
                {
                    response = "";   // FIX
                    last_char_time = millis();
                }
            }

            if (response.indexOf("ERROR") != -1 ||
                response.indexOf("+CME ERROR") != -1 ||
                response.indexOf("+CMS ERROR") != -1)
            {
                int res;
                printError(response, res);
                return false;
            }

            response = "";   // FIX
            last_char_time = millis();
        }
    }

    Debug_Print("<<< ⏱ TIMEOUT waiting for: ");
    Debug_Println(expected_response);
    Debug_Println(response);

    resp = response;
    return false;
}


bool Sim7600Manager::sendATCommandWithData(const char *cmd,
                                           const char *data,
                                           const char *expected_response,
                                           unsigned long timeout)
{
    clearSerialBuffer();

    Debug_Printf(">>> %s\n", cmd);
    _serial.println(cmd);

    unsigned long start_time = millis();
    String response = "";
    bool prompt_received = false;

    // Wait for '>' prompt
    while (millis() - start_time < timeout)
    {
        if (_serial.available())
        {
            char c = _serial.read();
            Serial.write(c);

            if (c == '>')
            {
                prompt_received = true;
                break;
            }
        }
        else
        {
            SIM7600_YIELD();
        }
    }

    if (!prompt_received)
    {
        Debug_Println("\n--- DID NOT RECEIVE '>' PROMPT ---");
        return false;
    }

    // send data WITHOUT newline
    Debug_Printf(">>> %s\n", data);
    _serial.print(data);   // <<< FIXED (was println)

    // wait for response
    response = "";
    start_time = millis();
    unsigned long last_char_time = millis();
    const unsigned long SETTLE_TIME = 100;

    while (millis() - start_time < timeout)
    {
        if (_serial.available())
        {
            char c = _serial.read();
            response += c;
            Serial.write(c);
            last_char_time = millis();
        }
        else
        {
            SIM7600_YIELD();
        }

        if (response.length() > 0 &&
            (millis() - last_char_time) >= SETTLE_TIME)
        {
            Debug_Println();

            if (response.indexOf("OK") != -1)
                return true;

            if (response.indexOf("ERROR") != -1 ||
                response.indexOf("+CME ERROR") != -1 ||
                response.indexOf("+CMS ERROR") != -1)
            {
                Debug_Println("--- ERROR RESPONSE ---");
                Debug_Println(response);
                return false;
            }

            last_char_time = millis();
        }
    }

    Debug_Println("\n--- TIMEOUT ---");
    Debug_Println(response);
    return false;
}

String Sim7600Manager::printError(const String &response, int &outCode)
{
    outCode = -1;

    if (response.indexOf("+CMQTT") != -1)
        return decodeMQTTError(response, outCode);

    if (response.indexOf("+HTTP") != -1)
        return decodeHTTPError(response, outCode);

    if (response.indexOf("+CIP") != -1)
        return decodeTCPError(response, outCode);

    // if (response.indexOf("+CSSL") != -1)
    //     return decodeSSLError(response, outCode);

    // Default fallback for unknown responses
    return decodeGeneralError(response, outCode);
}


String Sim7600Manager::decodeMQTTError(const String &response, int &outCode)
{
    int commaIndex = response.lastIndexOf(',');
    int colonIndex = response.lastIndexOf(':');
    int start = max(commaIndex, colonIndex);

    String numStr = (start != -1) ? response.substring(start + 1) : "";
    numStr.trim();
    outCode = numStr.toInt();

    const char *meaning = getSim7600ErrorString(SIM7600_ERR_MQTT, outCode);
    Debug_Printf("📡 MQTT → Code: %d → %s\n", outCode, meaning);
    return String(meaning);
}

String Sim7600Manager::decodeHTTPError(const String &response, int &outCode)
{
    int statusCode = 0, dataLen = 0;

    if (response.indexOf("+HTTPACTION:") != -1)
    {
        if (parseHttpActionResponse(response, statusCode, dataLen))
        {
            outCode = statusCode;
            const char *meaning = getSim7600ErrorString(SIM7600_ERR_HTTP, statusCode);
            Debug_Printf("🌐 HTTPACTION → Code: %d (%s), DataLen: %d bytes\n",
                          statusCode, meaning, dataLen);
            return String(meaning);
        }
    }

    // Fallback: simple +HTTPxxx: ,<errCode> style
    int commaIndex = response.lastIndexOf(',');
    if (commaIndex != -1)
    {
        String numStr = response.substring(commaIndex + 1);
        numStr.trim();
        outCode = numStr.toInt();
    }

    const char *meaning = getSim7600ErrorString(SIM7600_ERR_HTTP, outCode);
    Debug_Printf("🌐 HTTP → Code: %d → %s\n", outCode, meaning);
    return String(meaning);
}


String Sim7600Manager::decodeTCPError(const String &response, int &outCode)
{
    int commaIndex = response.lastIndexOf(',');
    int colonIndex = response.lastIndexOf(':');
    int start = max(commaIndex, colonIndex);

    String numStr = (start != -1) ? response.substring(start + 1) : "";
    numStr.trim();
    outCode = numStr.toInt();

    const char *meaning = getSim7600ErrorString(SIM7600_ERR_TCP, outCode);
    Debug_Printf("🔌 TCP → Code: %d → %s\n", outCode, meaning);
    return String(meaning);
}

String Sim7600Manager::decodeSSLError(const String &response, int &outCode)
{
    int commaIndex = response.lastIndexOf(',');
    int colonIndex = response.lastIndexOf(':');
    int start = max(commaIndex, colonIndex);

    String numStr = (start != -1) ? response.substring(start + 1) : "";
    numStr.trim();
    outCode = numStr.toInt();

    const char *meaning = getSim7600ErrorString(SIM7600_ERR_SSL, outCode);
    Debug_Printf("🔒 SSL → Code: %d → %s\n", outCode, meaning);
    return String(meaning);
}


String Sim7600Manager::decodeGeneralError(const String &response, int &outCode)
{
    int lastComma = response.lastIndexOf(',');
    int lastColon = response.lastIndexOf(':');
    int lastSep = max(lastComma, lastColon);

    if (lastSep != -1)
    {
        String numStr = response.substring(lastSep + 1);
        numStr.trim();
        int endIndex = numStr.indexOf('\r');
        if (endIndex != -1)
            numStr = numStr.substring(0, endIndex);
        numStr.trim();
        outCode = numStr.toInt();
    }
    else
    {
        // Fallback: find trailing digits
        for (int i = response.length() - 1; i >= 0; i--)
        {
            if (isDigit(response[i]))
            {
                int start = i;
                while (start > 0 && isDigit(response[start - 1]))
                    start--;
                String numStr = response.substring(start, i + 1);
                numStr.trim();
                outCode = numStr.toInt();
                break;
            }
        }
    }

    const char *meaning = getSim7600ErrorString(SIM7600_ERR_GENERAL, outCode);
    Debug_Printf("⚙️ GENERAL → Code: %d → %s\n", outCode, meaning);
    return String(meaning);
}


String Sim7600Manager::readResponse(const char* endMarker, unsigned long timeout) {
    String response = "";
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while (_serial.available()) {
            char c = _serial.read();
            response += c;
            // Stop reading once we see the marker or final "OK"
            if (response.indexOf(endMarker) != -1 || response.indexOf("OK") != -1) {
                return response;
            }
        }
        SIM7600_YIELD(); // Yield to scheduler
    }
    return response; // may be partial if timeout
}

int Sim7600Manager::extractLastCode(const String &response) {
    // Find the last comma in the response
    int lastComma = response.lastIndexOf(',');
    if (lastComma == -1) {
        // No comma found — maybe code is at end like "+CMQTTSTOP: 21"
        int colon = response.lastIndexOf(':');
        if (colon != -1) {
            String num = response.substring(colon + 1);
            num.trim();
            return num.toInt();
        }
        return -1;
    }

    // Extract substring after the last comma
    String numStr = response.substring(lastComma + 1);
    numStr.trim();

    // Convert to integer
    int code = numStr.toInt();
    return code;
}

bool Sim7600Manager::parseHttpActionResponse(const String &response, int &statusCode, int &dataLength)
{
    // Example valid response:
    // "OK\r\n+HTTPACTION: 0,200,3910544\r\n"
    
    int startIndex = response.indexOf("+HTTPACTION:");
    if (startIndex == -1) {
        Debug_Println("⚠️ No +HTTPACTION found in response");
        return false;
    }

    // Extract the substring that actually contains the numbers
    String sub = response.substring(startIndex);
    sub.trim();

    // Split by commas
    int firstComma = sub.indexOf(',');
    int secondComma = sub.indexOf(',', firstComma + 1);

    if (firstComma == -1 || secondComma == -1) {
        Debug_Println("⚠️ Malformed +HTTPACTION response");
        return false;
    }

    String statusStr = sub.substring(firstComma + 1, secondComma);
    String lenStr = sub.substring(secondComma + 1);

    statusStr.trim();
    lenStr.trim();

    statusCode = statusStr.toInt();
    dataLength = lenStr.toInt();

    // Debug print with meanings
    if (statusCode == 200)
        Debug_Printf("🌐 HTTPACTION → Code: %d (OK (Request succeeded)), DataLen: %d bytes\n", statusCode, dataLength);
    else
        Debug_Printf("🌐 HTTPACTION → Code: %d, DataLen: %d bytes\n", statusCode, dataLength);

    return true;
}

String Sim7600Manager::getLastResponse(unsigned long timeout)
{
    String response = "";
    unsigned long start = millis();

    while (millis() - start < timeout)
    {
        while (_serial.available())
        {
            char c = _serial.read();
            response += c;
        }

        // Stop if "OK" or "ERROR" appears
        if (response.indexOf("OK") != -1 || response.indexOf("ERROR") != -1)
            break;

        SIM7600_YIELD(); // Yield to scheduler
    }

    response.trim();
    return response;
}
