# SIM7600Lib
A lightweight and reusable library for ESP32/Arduino to manage SIM7600 LTE modules. Supports HTTP, MQTT, NTP, SMS, and general communication features, making SIM7600 integration simple and reliable.

This is a professionally structured `README.md` for your GitHub repository. It uses clear headings, badges, code snippets, and icons to make the library look high-quality and easy to use.

---
Here is the updated **README.md**. I have added a dedicated **"Examples & Usage"** section that breaks down the three core scenarios (Basic, MQTT, and HTTP/OTA) with clear explanations of how the code works.

---

# 🛰️ SIM7600-Master 

**A robust, modular, and easy-to-use Arduino library for the SIM7600 series LTE modems.**

This library provides a high-level interface for GPRS connectivity, Secure MQTT, HTTP(S) requests, NTP time synchronization, and over-the-air (OTA) firmware updates. It features a built-in error decoder and is designed to work seamlessly with ESP32 and other Arduino-compatible boards.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)

---

## ✨ Features

- 📶 **Network Management**: Automatic GPRS attachment and signal quality monitoring.
- ✉️ **MQTT/MQTTS**: Support for multiple clients, Last Will, and asynchronous callbacks.
- 🌐 **HTTP/HTTPS**: Simple GET requests and automated OTA updates via cellular.
- 🕒 **NTP Sync**: Fetch real-time clock data from global servers.
- 🛠️ **Error Decoder**: Converts cryptic AT command error codes into human-readable strings.
- 🧵 **FreeRTOS Ready**: Built-in support for `vTaskDelay` and non-blocking yields.

---

## 🚀 Installation

1. Download this repository as a `.zip`.
2. In the Arduino IDE, go to **Sketch** -> **Include Library** -> **Add .ZIP Library...**.
3. Ensure you are using an ESP32 for OTA features, though basic functions work on most Arduino boards.

---

## 📂 Examples & Usage

The library includes three primary examples to get you started quickly. You can find these in the `examples/` folder.

### 1. Basic Connection & Network
**File:** `01_Basic_Connection.ino`  
This example shows how to initialize the hardware and establish a GPRS data connection.

*   **Initialization:** Set the baud rate and hardware pins (RX/TX).
*   **Signal Monitoring:** Uses `modem.signalQuality()` to return a value from 0-31.
*   **GPRS:** Attaches the modem to your provider's APN to enable internet access.

```cpp
if (modem.begin(115200, SERIAL_8N1, 16, 17)) {
    modem.connectGPRS("your_apn"); 
}
```

### 2. MQTT with Callbacks
**File:** `02_MQTT_Client.ino`  
This example demonstrates a fully asynchronous MQTT client.

*   **Callbacks:** You register a function (e.g., `onMessageReceived`) that triggers automatically when a message arrives.
*   **The Loop:** You **must** call `modem.mqtt.loop()` in your main `void loop()`. This allows the library to process incoming serial data from the modem.
*   **Publish/Subscribe:** Standard MQTT operations with support for QoS and Retain flags.

```cpp
void setup() {
    modem.mqtt.setCallback(onMessageReceived);
    modem.mqtt.connect("broker.hivemq.com", 1883, "ClientID");
}

void loop() {
    modem.mqtt.loop(); // Important! Handles incoming data
}
```

### 3. HTTPS & OTA Updates
**File:** `03_HTTPS_and_OTA.ino`  
Learn how to interact with web APIs and update firmware remotely.

*   **HTTPS GET:** Simple method to fetch JSON or HTML content from a URL.
*   **NTP Time:** Synchronizes the modem's internal clock with global time servers.
*   **OTA (Over-The-Air):** Downloads a `.bin` file from a server and flashes the ESP32 directly. The `OTAState` struct tracks progress (0-100%) and status messages.

```cpp
OTAState ota;
if (modem.http.performOTA("http://myserver.com/v2.bin", ota)) {
    Serial.println("Update Complete!");
    ESP.restart();
}
```

---

## 🛠️ Integrated Error Decoder
One of the most powerful features of this library is the **Automatic Error Decoder**. When an AT command fails, the library intercepts the error code (like `+CMQTT: 19`) and translates it into a human-readable message.

**Example Console Output:**
> `>>> AT+CMQTTCONNECT=0,...`  
> `<<< ERROR`  
> `📡 MQTT → Code: 19 → Client is already in use`

---

## 🔌 Hardware Connection
The SIM7600 requires significant power during LTE transmission. Ensure your power supply can handle **2A spikes**.

| SIM7600 Pin | ESP32 Pin | Note |
| :--- | :--- | :--- |
| **TX** | RX (GPIO 16) | 3.3V Logic Level |
| **RX** | TX (GPIO 17) | 3.3V Logic Level |
| **GND** | GND | Common Ground |
| **VCC** | 5V / VBAT | Min 2A Peak Current |

---

## ⚙️ Configuration
If you are using **FreeRTOS** (standard on ESP32), the library is optimized to yield to the scheduler during wait times. To enable this, ensure `SIM7600_USE_FREERTOS` is defined (this is handled automatically in most ESP32 environments).

---

## 📄 License
Distributed under the MIT License. See `LICENSE` for more information.

---
**Developed with ❤️ for the IoT community.**