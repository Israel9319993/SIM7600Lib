// SIM7600_NTP.h
#ifndef SIM7600_NTP_H
#define SIM7600_NTP_H

#include <Arduino.h>

class Sim7600Manager; // Forward declaration

class Sim7600NTP
{
public:
    Sim7600NTP(Sim7600Manager &manager);

    bool fetchNTPTime(const char *ntpServer, int timezone, String &outTime);

private:
    Sim7600Manager &_manager;
};

#endif // SIM7600_NTP_H
