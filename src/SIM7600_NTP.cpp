// SIM7600_NTP.cpp
#include "SIM7600_NTP.h"
#include "SIM7600.h" // Include main manager header

Sim7600NTP::Sim7600NTP(Sim7600Manager &manager) : _manager(manager) {}

bool Sim7600NTP::fetchNTPTime(const char *ntpServer, int timezone, String &outTime)
{
    char cmd_buffer[100];
    char response_buffer[100];

    Debug_Println("\n--- Fetching NTP Time ---");

    // Step 1: Configure the NTP server and timezone
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CNTP=\"%s\",%d", ntpServer, timezone);
    if (!_manager.sendATCommand(cmd_buffer, "OK", 5000))
    {
        Debug_Println("❌ Failed to configure NTP server");
        return false;
    }

    SIM7600_DELAY(5000);

    // Step 2: Start NTP synchronization
    if (!_manager.sendATCommand("AT+CNTP", "+CNTP: 1", 15000))
    {
        Debug_Println("❌ Failed to sync with NTP server");
        return false;
    }

    SIM7600_DELAY(2000);

    // Step 3: Read the updated time from the modem
    // BUG FIX: use the overload that captures the response, since sendATCommand consumes serial data
    String rawResponse;
    if (!_manager.sendATCommand("AT+CCLK?", "+CCLK:", 3000, rawResponse))
    {
        Debug_Println("❌ Failed to read clock from modem");
        return false;
    }

    // Step 4: Extract the time string from response
    int startIdx = rawResponse.indexOf("\"");
    int endIdx = rawResponse.lastIndexOf("\"");
    if (startIdx == -1 || endIdx == -1 || startIdx == endIdx)
    {
        Debug_Println("❌ Invalid time format in response");
        return false;
    }

    outTime = rawResponse.substring(startIdx + 1, endIdx);
    Debug_Print("✅ NTP Time Fetched: ");
    Debug_Println(outTime);

    return true;
}
