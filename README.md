# 🛡️ ESP32 Safety Monitor

An IoT safety monitoring system built on the ESP32 that detects fire, gas leaks, temperature, and humidity — and serves a live web dashboard over Wi-Fi.

![Board Photo](images/board.jpg)

## Features

- 🌡️ Temperature & humidity monitoring (DHT11)
- 💨 Gas level detection (MQ-3 analog sensor)
- 🔥 Flame/fire detection (digital flame sensor)
- 🔔 Buzzer alarm on danger detection
- 🌐 Live web dashboard hosted from the ESP32 — no external server needed
- Auto-refreshes every 2 seconds

## Hardware

| Component       | Quantity |
|----------------|----------|
| ESP32 DevKit   | 1        |
| DHT11 sensor   | 1        |
| MQ-3 gas sensor | 1       |
| Flame sensor module | 1   |
| Passive buzzer | 1        |
| 10kΩ resistor  | 1 (pull-up for DHT11 DATA) |
| Perfboard 7×9 cm | 1     |
| Jumper wires   | Several  |

## Wiring

See [`wiring-diagram.svg`](wiring-diagram.svg) for the full pin diagram.

| Component | ESP32 Pin |
|-----------|-----------|
| DHT11 DATA | GPIO 21 |
| DHT11 VCC  | 3.3 V   |
| DHT11 GND  | GND     |
| Flame sensor DO | GPIO 22 |
| Buzzer I/O | GPIO 23 |
| MQ-3 AO   | GPIO 34 (ADC) |
| MQ-3 VCC  | VIN (5 V) |

## Software

- **IDE:** Arduino IDE 2.x
- **Board package:** ESP32 by Espressif
- **Libraries:**
  - `ESPAsyncWebServer`
  - `AsyncTCP`
  - `DHT sensor library` by Adafruit

## Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support via Boards Manager
3. Install the libraries listed above via Library Manager
4. Open `src/safety_monitor.ino`
5. Update `ssid` and `password` with your Wi-Fi credentials
6. Upload to your ESP32
7. Open Serial Monitor at 115200 baud to find the IP address
8. Visit `http://<IP>` from any device on the same network

## Thresholds

| Sensor | Danger Threshold |
|--------|-----------------|
| Gas (MQ-3) | ≥ 2000 / 4095 |
| Temperature | ≥ 50 °C |
| Flame | Any digital HIGH |

## Tech Stack

ESP32 · Arduino IDE · C++ · HTML/CSS · Wi-Fi · Embedded Systems
