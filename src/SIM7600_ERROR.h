#ifndef SIM7600_ERROR_H
#define SIM7600_ERROR_H

#include <Arduino.h>

enum SIM7600ErrorCategory {
    SIM7600_ERR_GENERAL,
    SIM7600_ERR_MQTT,
    SIM7600_ERR_HTTP,
    SIM7600_ERR_TCP,
    SIM7600_ERR_SSL
};

// 👇 Only declare here
const char *getSim7600ErrorString(SIM7600ErrorCategory category, int errCode);

#endif
