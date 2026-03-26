#ifndef SIM7600_HTTP_H
#define SIM7600_HTTP_H

#include <Arduino.h>

class Sim7600Manager;

enum OTAStatus {
    OTA_IDLE,
    OTA_STARTING,
    OTA_HTTP_INIT,
    OTA_DOWNLOADING,
    OTA_WRITING,
    OTA_COMPLETED,
    OTA_FAILED,
    OTA_ABORTED
};

struct OTAState {
    OTAStatus status = OTA_IDLE;
    String message = "";
    int progress = 0; // 0 - 100 %
    bool success = false;
};

class Sim7600HTTP
{
public:
    // Constructor needs a reference to the main manager
    Sim7600HTTP(Sim7600Manager &manager);

    // --- Public HTTP Functions ---
    String httpsGET(const char *url);

    bool performOTA(const char* firmwareUrl, OTAState &ota);

private:
    Sim7600Manager &_manager; // Reference to the parent manager
    
};

#endif





// bool Sim7600HTTP::performOTA(const char* firmwareUrl, int &progress, const char* state, const char* response) {
//     Debug_Println("\n🚀 Starting SIM7600 OTA Update...");
//     char cmd_buffer[256];
//      ota.status = OTA_STARTING;
//     ota.message = "Starting OTA process...";
//     ota.progress = 0;
//     ota.success = false;

//     // Terminate previous HTTP session
//     _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//     delay(200);

//     // Initialize HTTP
//     if (!_manager.sendATCommand("AT+HTTPINIT", "OK", 3000)) {
//         Debug_Println("❌ HTTPINIT failed");
//         return false;
//     }

//     // Set HTTP timeouts
//     _manager.sendATCommand("AT+HTTPPARA=\"RECVTO\",120", "OK", 1000);
//     _manager.sendATCommand("AT+HTTPPARA=\"RESPTO\",120", "OK", 1000);

//     // Set URL
//     snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+HTTPPARA=\"URL\",\"%s\"", firmwareUrl);
//     if (!_manager.sendATCommand(cmd_buffer, "OK", 5000)) {
//         Debug_Println("❌ Failed to set HTTP URL");
//         _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//         return false;
//     }

//     // Start HTTP GET request
//     String actionResp;
//     if (!_manager.sendATCommand("AT+HTTPACTION=0", "+HTTPACTION:", 60000, actionResp)) {
//         Debug_Println("❌ HTTPACTION timeout");
//         _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//         return false;
//     }

//     // Parse HTTP response code and content length
//     int httpCode = 0, dataLen = 0;
//     if (!_manager.parseHttpActionResponse(actionResp, httpCode, dataLen) || httpCode != 200) {
//         Debug_Printf("❌ HTTP failed (Code %d)\n", httpCode);
//         _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//         return false;
//     }

//     Debug_Printf("🌐 HTTP 200 OK, Size: %d bytes\n", httpCode, dataLen);

//     // Begin OTA update
//     if (!Update.begin(dataLen)) {
//         Debug_Printf("❌ OTA begin failed: %s\n", Update.errorString());
//         _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//         return false;
//     }

//     Debug_Println("⬇️  Downloading firmware...");

//     // Download parameters
//     const int CHUNK_SIZE = 1024;
//     uint8_t buffer[CHUNK_SIZE];
//     int totalBytesRead = 0;
//     int consecutiveErrors = 0;
//     const int MAX_ERRORS = 10;

//     while (totalBytesRead < dataLen) {
//         // Request next chunk
//         int bytesToRead = min(CHUNK_SIZE, dataLen - totalBytesRead);
//         snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+HTTPREAD=%d,%d", totalBytesRead, bytesToRead);

//         // Clear UART buffer
//         delay(20);
//         while (_manager._serial.available()) {
//             _manager._serial.read();
//         }

//         // Send command
//         _manager._serial.println(cmd_buffer);
        
//         // Parse response - look for +HTTPREAD: DATA,<len>
//         unsigned long timeout = millis() + 10000;
//         String header = "";
//         bool foundHeader = false;
//         int expectedLen = 0;

//         while (millis() < timeout && !foundHeader) {
//             if (_manager._serial.available()) {
//                 char c = _manager._serial.read();
//                 header += c;

//                 // Look for header
//                 int idx = header.indexOf("+HTTPREAD: DATA,");
//                 if (idx >= 0) {
//                     int commaIdx = idx + 16; // Length of "+HTTPREAD: DATA,"
//                     int endIdx = header.indexOf("\r", commaIdx);
//                     if (endIdx == -1) endIdx = header.indexOf("\n", commaIdx);
                    
//                     if (endIdx > commaIdx) {
//                         String lenStr = header.substring(commaIdx, endIdx);
//                         lenStr.trim();
//                         expectedLen = lenStr.toInt();
                        
//                         if (expectedLen > 0 && expectedLen <= CHUNK_SIZE) {
//                             foundHeader = true;
                            
//                             // Consume remaining header bytes until newline
//                             while (_manager._serial.available()) {
//                                 if (_manager._serial.read() == '\n') break;
//                             }
//                             break;
//                         }
//                     }
//                 }

//                 // Prevent header buffer from growing too large
//                 if (header.length() > 150) {
//                     header = header.substring(header.length() - 100);
//                 }
//             }
//         }

//         if (!foundHeader) {
//             Debug_Printf("❌ No header at offset %d\n", totalBytesRead);
//             consecutiveErrors++;
//             if (consecutiveErrors >= MAX_ERRORS) {
//                 Update.abort();
//                 _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//                 return false;
//             }
//             delay(500);
//             continue;
//         }

//         // Read binary data directly into buffer
//         int bytesRead = 0;
//         timeout = millis() + 10000;

//         while (bytesRead < expectedLen && millis() < timeout) {
//             if (_manager._serial.available()) {
//                 buffer[bytesRead++] = _manager._serial.read();
//             }
//         }

//         if (bytesRead != expectedLen) {
//             Debug_Printf("⚠️ Expected %d, got %d bytes\n", expectedLen, bytesRead);
//         }

//         if (bytesRead > 0) {
//             // Write to flash
//             size_t written = Update.write(buffer, bytesRead);
//             if (written != bytesRead) {
//                 Debug_Printf("❌ Flash write failed: %d/%d\n", written, bytesRead);
//                 Update.abort();
//                 _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//                 return false;
//             }

//             totalBytesRead += bytesRead;
//             consecutiveErrors = 0;

//             // Progress every 50KB
//             if (totalBytesRead % 51200 < bytesRead || totalBytesRead == dataLen) {
//                 Debug_Printf("📦 %d / %d bytes (%.1f%%)\n", 
//                     totalBytesRead, dataLen, (totalBytesRead * 100.0) / dataLen);
//             }
//         } else {
//             consecutiveErrors++;
//             if (consecutiveErrors >= MAX_ERRORS) {
//                 Debug_Println("❌ Too many errors");
//                 Update.abort();
//                 _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);
//                 return false;
//             }
//         }

//         // Minimal delay for speed
//         delay(10);
//     }

//     _manager.sendATCommand("AT+HTTPTERM", "OK", 2000);

//     if (totalBytesRead < dataLen) {
//         Debug_Printf("❌ Incomplete: %d/%d bytes\n", totalBytesRead, dataLen);
//         Update.abort();
//         return false;
//     }

//     // Finalize OTA
//     if (Update.end(true)) {
//         if (Update.hasError()) {
//             Debug_Printf("❌ OTA failed: %s\n", Update.errorString());
//             return false;
//         }
//         Debug_Println("✅ OTA Success! Rebooting...");
//         delay(1000);
//         ESP.restart();
//         return true;
//     }

//     Debug_Printf("❌ Update.end() failed: %s\n", Update.errorString());
//     return false;
// }