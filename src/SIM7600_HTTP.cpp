#include "SIM7600_HTTP.h"
#include "SIM7600.h"
#include <Update.h>

Sim7600HTTP::Sim7600HTTP(Sim7600Manager &manager) : _manager(manager) {}

// --- HTTP Functions Implementation ---

String Sim7600HTTP::httpsGET(const char *url)
{
    Debug_Println("\n--- Starting HTTPS GET Request ---");
    char cmd_buffer[256];
    _manager.sendATCommand("AT+HTTPTERM", "OK", 5000); // Reset state
    if (!_manager.sendATCommand("AT+HTTPINIT", "OK", 10000))
        return "";

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+HTTPPARA=\"URL\",\"%s\"", url);
    if (!_manager.sendATCommand(cmd_buffer, "OK", 5000))
    {
        _manager.sendATCommand("AT+HTTPTERM", "OK", 5000);
        return "";
    }

    if (!_manager.sendATCommand("AT+HTTPACTION=0", "OK", 10000))
    {
        _manager.sendATCommand("AT+HTTPTERM", "OK", 5000);
        return "";
    }

    String response = "";
    unsigned long start = millis();
    while (millis() - start < 20000)
    {
        if (_manager._serial.available())
            response += _manager._serial.readString();
        if (response.indexOf("+HTTPACTION:") != -1)
            break;
    }

    if (response.indexOf("200") == -1)
    { // Check for success code
        _manager.sendATCommand("AT+HTTPTERM", "OK", 5000);
        return "";
    }

    if (!_manager.sendATCommand("AT+HTTPREAD=0,2048", "OK", 10000))
    { // Read up to 2KB
        _manager.sendATCommand("AT+HTTPTERM", "OK", 5000);
        return "";
    }

    response = ""; // Reuse response string for the content
    start = millis();
    while (millis() - start < 5000)
    {
        if (_manager._serial.available())
            response += _manager._serial.readString();
    }

    _manager.sendATCommand("AT+HTTPTERM", "OK", 5000);

    int jsonStart = response.indexOf('{');
    if (jsonStart != -1)
    {
        return response.substring(jsonStart);
    }
    return ""; // Return empty if no JSON found
}

bool Sim7600HTTP::performOTA(const char *firmwareUrl, OTAState &ota)
{
    ota.status = OTA_STARTING;
    ota.message = "Starting OTA process..";
    ota.progress = 0;
    ota.success = false;

    Debug_Println("\n🚀 Starting SIM7600 OTA Update...");
    char cmd_buffer[256];

    _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
    SIM7600_DELAY(200);

    if (!_manager.sendATCommand("AT+HTTPINIT", "OK", 3000))
    {
        ota.status = OTA_FAILED;
        ota.message = "HTTPINIT failed";
        Debug_Println("❌ HTTPINIT failed");
        return false;
    }
    ota.status = OTA_HTTP_INIT;
    ota.message = "HTTP initialized";

    // Set HTTP timeouts
    _manager.sendATCommand("AT+HTTPPARA=\"RECVTO\",120", "OK", 1000);
    _manager.sendATCommand("AT+HTTPPARA=\"RESPTO\",120", "OK", 1000);

    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+HTTPPARA=\"URL\",\"%s\"", firmwareUrl);
    if (!_manager.sendATCommand(cmd_buffer, "OK", 5000))
    {
        ota.status = OTA_FAILED;
        ota.message = "Failed to set URL";
        _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    ota.message = "Starting HTTP GET...";
    String actionResp;
    if (!_manager.sendATCommand("AT+HTTPACTION=0", "+HTTPACTION:", 60000, actionResp))
    {
        ota.status = OTA_FAILED;
        ota.message = "HTTP timeout";
        _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    int httpCode = 0, dataLen = 0;
    if (!_manager.parseHttpActionResponse(actionResp, httpCode, dataLen) || httpCode != 200)
    {
        ota.status = OTA_FAILED;
        ota.message = "HTTP resp error";
        Debug_Printf("❌ HTTP failed (Code %d)\n", httpCode);
        _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    Debug_Printf("🌐 HTTP 200 OK, Size: %d bytes\n", dataLen);
    ota.status = OTA_DOWNLOADING;
    ota.message = "Downloading firmware";
    ota.progress = 0;

    if (!Update.begin(dataLen))
    {
        ota.status = OTA_FAILED;
        ota.message = "OTA begin failed";
        Debug_Printf("❌ OTA begin failed: %s\n", Update.errorString());
        _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    Debug_Println("⬇️  Downloading firmware");
    const int CHUNK_SIZE = 1024;
    uint8_t buffer[CHUNK_SIZE];
    int totalBytesRead = 0;
    int consecutiveErrors = 0;
    const int MAX_ERRORS = 10;

    while (totalBytesRead < dataLen)
    {
        // ... (same chunk download logic as your code)
        // after each chunk written:
        // Request next chunk
        int bytesToRead = min(CHUNK_SIZE, dataLen - totalBytesRead);
        snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+HTTPREAD=%d,%d", totalBytesRead, bytesToRead);

        // Clear UART buffer
        SIM7600_DELAY(20);
        while (_manager._serial.available())
        {
            _manager._serial.read();
        }

        // Send command
        _manager._serial.println(cmd_buffer);

        // Parse response - look for +HTTPREAD: DATA,<len>
        unsigned long timeout = millis() + 10000;
        String header = "";
        bool foundHeader = false;
        int expectedLen = 0;

        while (millis() < timeout && !foundHeader)
        {
            if (_manager._serial.available())
            {
                char c = _manager._serial.read();
                header += c;

                // Look for header
                int idx = header.indexOf("+HTTPREAD: DATA,");
                if (idx >= 0)
                {
                    int commaIdx = idx + 16; // Length of "+HTTPREAD: DATA,"
                    int endIdx = header.indexOf("\r", commaIdx);
                    if (endIdx == -1)
                        endIdx = header.indexOf("\n", commaIdx);

                    if (endIdx > commaIdx)
                    {
                        String lenStr = header.substring(commaIdx, endIdx);
                        lenStr.trim();
                        expectedLen = lenStr.toInt();

                        if (expectedLen > 0 && expectedLen <= CHUNK_SIZE)
                        {
                            foundHeader = true;

                            // Consume remaining header bytes until newline
                            while (_manager._serial.available())
                            {
                                if (_manager._serial.read() == '\n')
                                    break;
                            }
                            break;
                        }
                    }
                }

                // Prevent header buffer from growing too large
                if (header.length() > 150)
                {
                    header = header.substring(header.length() - 100);
                }
            }
        }

        if (!foundHeader)
        {
            Debug_Printf("❌ No header at offset %d\n", totalBytesRead);
            consecutiveErrors++;
            if (consecutiveErrors >= MAX_ERRORS)
            {
                Update.abort();
                _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
                return false;
            }
            SIM7600_DELAY(500);
            continue;
        }

        // Read binary data directly into buffer
        int bytesRead = 0;
        timeout = millis() + 10000;

        while (bytesRead < expectedLen && millis() < timeout)
        {
            if (_manager._serial.available())
            {
                buffer[bytesRead++] = _manager._serial.read();
            }
        }

        if (bytesRead != expectedLen)
        {
            Debug_Printf("⚠️ Expected %d, got %d bytes\n", expectedLen, bytesRead);
        }

        if (bytesRead > 0)
        {
            // Write to flash
            size_t written = Update.write(buffer, bytesRead);
            if (written != bytesRead)
            {
                Debug_Printf("❌ Flash write failed: %d/%d\n", written, bytesRead);
                Update.abort();
                _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
                return false;
            }

            totalBytesRead += bytesRead;
            consecutiveErrors = 0;

            // Progress every 50KB
            if (totalBytesRead % 51200 < bytesRead || totalBytesRead == dataLen)
            {
                Debug_Printf("📦 %d / %d bytes (%.1f%%)\n",
                              totalBytesRead, dataLen, (totalBytesRead * 100.0) / dataLen);
                ota.progress = (totalBytesRead * 100) / dataLen;
                ota.status = OTA_WRITING;
            }
        }
        else
        {
            consecutiveErrors++;
            if (consecutiveErrors >= MAX_ERRORS)
            {
                Debug_Println("❌ Too many errors");
                Update.abort();
                _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
                ota.status = OTA_FAILED;
                ota.message = "Incomplete download";
                return false;
            }
        }

        // Minimal delay for speed
        SIM7600_DELAY(10);
    }

    _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);

    if (totalBytesRead < dataLen)
    {
        ota.status = OTA_FAILED;
        ota.message = "Incomplete download";
        Update.abort();
        return false;
    }

    if (Update.end(true))
    {
        if (Update.hasError())
        {
            ota.status = OTA_FAILED;
            ota.message = String("failed: ") + Update.errorString();
            return false;
        }
        ota.status = OTA_COMPLETED;
        ota.message = "OTA success! Rebooting...";
        ota.success = true;
        ota.progress = 100;
        Debug_Println("✅ OTA Success! Rebooting...");
        SIM7600_DELAY(1000);
      
        return true;
    }

    ota.status = OTA_FAILED;
    ota.message = String("Update.end() failed: ") + Update.errorString();
    return false;
}
