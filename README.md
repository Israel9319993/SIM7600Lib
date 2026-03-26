# SIM7600Lib
A lightweight and reusable library for ESP32/Arduino to manage SIM7600 LTE modules. Supports HTTP, MQTT, NTP, SMS, and general communication features, making SIM7600 integration simple and reliable.

This is a professionally structured `README.md` for your GitHub repository. It uses clear headings, badges, code snippets, and icons to make the library look high-quality and easy to use.

---

# 🛰️ SIM7600-Master 

**A robust, modular, and easy-to-use Arduino library for the SIM7600 series LTE modems.**

This library provides a high-level interface for GPRS connectivity, Secure MQTT, HTTP(S) requests, NTP time synchronization, and over-the-air (OTA) firmware updates. It features a built-in error decoder and is designed to work seamlessly with ESP32 and other Arduino-compatible boards.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)

---

## ✨ Features

- 📶 **Network Management**: Automatic GPRS attachment and signal quality monitoring.
- ✉️ **MQTT/MQTTS**: Support for multiple clients, Last Will, clean sessions, and SSL.
- 🌐 **HTTP/HTTPS**: Simple GET requests and automated OTA updates.
- 🕒 **NTP Sync**: Fetch real-time clock data from global servers.
- 🛠️ **Error Decoder**: Converts cryptic AT command error codes into human-readable strings.
- 🧵 **FreeRTOS Ready**: Built-in support for `vTaskDelay` and non-blocking yields.

---

## 🚀 Quick Start

### 1. Installation
1. Download this repository as a `.zip`.
2. In the Arduino IDE, go to **Sketch** -> **Include Library** -> **Add .ZIP Library...**.

### 2. Basic Setup
```cpp
#include <SIM7600.h>

// Use HardwareSerial 2 for SIM7600
Sim7600Manager modem(Serial2);

void setup() {
    Serial.begin(115200);
    
    // Initialize Modem (Baud, Config, RX, TX)
    if (modem.begin(115200, SERIAL_8N1, 16, 17)) {
        Serial.println("Modem Ready!");
        modem.connectGPRS("your_apn_here");
    }
}

void loop() {
    // Check signal every 20 seconds automatically
    int rssi = modem.signalQuality();
    Serial.printf("Signal Strength: %d/31\n", rssi);
    delay(5000);
}
```

---

## 📑 Module Documentation

### ☁️ MQTT Support (`modem.mqtt`)
The MQTT module handles asynchronous messaging and state management.

| Method | Description |
| :--- | :--- |
| `connect()` | Connect to a standard MQTT broker. |
| `connectSecure()` | Connect using SSL/TLS. |
| `subscribe(topic, qos)` | Subscribe to a specific topic. |
| `publish(topic, msg, retained, qos)` | Send a message to the broker. |
| `setCallback(cb)` | Register a function to handle incoming messages. |

**Example Callback:**
```cpp
void onMqttMessage(const String& topic, const String& payload) {
    Serial.println("Message arrived: " + payload);
}

void setup() {
    modem.mqtt.setCallback(onMqttMessage);
}
```

---

### 📥 HTTP & OTA Updates (`modem.http`)
Perform secure web requests or update your ESP32 firmware over the cellular network.

```cpp
// Simple HTTPS GET
String response = modem.http.httpsGET("https://api.example.com/data");

// Perform OTA Update
OTAState state;
if (modem.http.performOTA("http://server.com/firmware.bin", state)) {
    Serial.println("Update success! Rebooting...");
    ESP.restart();
}
```

---

### ⏰ NTP Time (`modem.ntp`)
Keep your system clock accurate without a GPS module.

```cpp
String currentTime;
if (modem.ntp.fetchNTPTime("pool.ntp.org", 0, currentTime)) {
    Serial.println("Network Time: " + currentTime);
}
```

---

## 🛠️ Advanced Error Handling
Stop guessing why commands fail. The library includes an integrated error interpreter for MQTT, HTTP, and TCP.

```cpp
// Internally used during failures
// Example output: "📡 MQTT → Code: 19 → Client is already in use"
```

---

## 🔌 Hardware Connection
For optimal performance, ensure your SIM7600 has a dedicated power source (3.7V - 4.2V) capable of providing at least **2A current spikes**.

| SIM7600 Pin | ESP32 Pin | Note |
| :--- | :--- | :--- |
| **TX** | RX (GPIO 16) | 3.3V Logic Level |
| **RX** | TX (GPIO 17) | 3.3V Logic Level |
| **GND** | GND | Common Ground |
| **VCC** | 5V / VBAT | Min 2A Peak |

---

## ⚙️ Configuration
If using **FreeRTOS**, define `SIM7600_USE_FREERTOS` in your build flags or before including the header to ensure the library uses non-blocking delays.

```cpp
#define SIM7600_USE_FREERTOS
#include <SIM7600.h>
```

---

## 🤝 Contributing
1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License
Distributed under the MIT License. See `LICENSE` for more information.

---
**Developed with ❤️ for the IoT community.**
