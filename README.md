# AetherIoT: Unified Zero-Heap Industrial IoT Framework

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![PlatformIO Registry](https://img.shields.io/badge/PlatformIO-AetherIoT-orange.svg)](https://registry.platformio.org/)
[![ESP-IDF Component](https://img.shields.io/badge/ESP--IDF-Component-green.svg)](https://components.espressif.com/)
[![Arduino Library](https://img.shields.io/badge/Arduino-Library-00979D.svg)](https://www.arduino.cc/reference/en/libraries/)
[![Zero-Heap](https://img.shields.io/badge/Dynamic%20Memory-0%25%20Heap-brightgreen.svg)]()
[![Documentation](https://img.shields.io/badge/Docs-GitHub%20Pages-blue.svg)](https://dhimasardinata.github.io/AetherIoT/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

**[English](README.md) | [Bahasa Indonesia](README.id.md) | 🌐 [Interactive Documentation Website](https://dhimasardinata.github.io/AetherIoT/)**

**AetherIoT** is an ultra-reliable, high-performance, deterministic C++20 bare-metal framework designed for **Industrial IoT, Precision Hydroponics, Smart Agriculture, Remote Weather Stations, High-Density Multi-Servo Actuation, and Telemetry Gateways** across **all Espressif silicon targets** (ESP32 Classic, S2, S3, C2, C3, C6, H2, P4, ESP8266).

---

## ⚡ 5-Line Quickstart

Get up and running in seconds with zero boilerplate:

```cpp
#include "iot_framework.hpp"

IOT_APP(app) {
    // 1. Declarative Safety Guard
    iot::guard("Suhu").max(35.0f).on_breach([](auto, auto v) {
        iot::log_error("ALERT: Temperature %.1f C exceeded safe limit!", v);
    });

    // 2. Real-Time Telemetry Stream
    app.on_data([](auto d) {
        iot::log_info("Temp: %.1f C | Humidity: %.1f %%RH | pH: %.2f", d.temp, d.hum, d.ph);
    });
}
```

---

## 🎯 Supported Espressif Silicon Matrix

| Target SoC | Architecture & Cores | Key Capabilities Supported | Recommended Use Cases |
| :--- | :--- | :--- | :--- |
| **ESP32 Classic** | Xtensa Dual-Core LX6 (240MHz) | Wi-Fi 4, BLE 4.2, BT Classic SPP, ULP | Hydroponics Gateways, RS-485 Modbus Hubs |
| **ESP32-S3** | Xtensa Dual-Core LX7 (240MHz) | Vector AI Instructions, USB OTG, Octal PSRAM | Edge AI Vision, Real-Time High-Speed DSP |
| **ESP32-S2** | Xtensa Single-Core LX7 (240MHz) | Native USB OTG CDC, Ultra-Low Deep Sleep Power | USB Industrial Dongles, Portable Dataloggers |
| **ESP32-C3** | 32-bit RISC-V (160MHz) | Wi-Fi 4, BLE 5.0 Long Range, Hardware Crypto | Cost-Effective Smart Agriculture Nodes |
| **ESP32-C6** | 32-bit RISC-V (160MHz) | **Wi-Fi 6**, BLE 5.0, **Thread / Zigbee 802.15.4** | Modern Smart Farms, Matter Mesh Bridges |
| **ESP32-H2** | 32-bit RISC-V (96MHz) | **Thread / Zigbee / Matter 802.15.4**, BLE 5.3 | Ultra-Low Power Matter Mesh Sensors |
| **ESP32-P4** | Dual-Core RISC-V High-Performance | Camera/Display MIPI Interface, HW H.264 | Industrial Smart HMI Touch Dashboards |
| **ESP8266** | Xtensa L106 (80/160MHz) | Wi-Fi 4, Low-Cost UART Telemetry | Legacy Ultra-Low-Cost Remote Nodes |
| **Host Native** | x86_64 / ARM64 (Linux/macOS/Win) | Full C++20 Simulation & Mock Peripherals | CI/CD Pipelines, Automated Unit Testing |

---

## 💎 Core Architecture & Capabilities

1. **Deterministic Zero-Heap Architecture (0% Dynamic Allocation)**:
   - Eliminates heap fragmentation and OOM reboots by allocating all buffers, circular queues, telemetry packets, and crypto states in static memory and stack.
2. **High-Density Multi-Servo Matrix (Relay-Like API)**:
   - Control up to 32 servos with declarative relay-style operations: `iot::servo_open("GateValve1")`, `iot::servo_close("GateValve1")`, `iot::servo_toggle(0)`, `iot::servo_is_open(0)`.
3. **High-Density Sensor Multiplexing**:
   - **TCA9548A 8-Channel I2C MUX**: Connect multiple identical I2C devices (`iot::i2c_channel(ch)`).
   - **1-Wire Multi-Drop Temperature Array**: Read 16-32x DS18B20 sensors on 1 single GPIO (`iot::read_temp_probe(idx)`).
   - **CD74HC4067 16-Channel Analog MUX / ADS1115 16-Bit ADC**: Expand analog inputs (`iot::read_analog_mux_voltage(ch)`).
   - **Modbus Multi-Slave Hub**: Auto-poll up to 32 Modbus RTU sensors on one RS-485 bus.
4. **Multi-Scheme Payload Encryption & Anti-Replay Protection**:
   - `AES_256_CBC` (Anti-Replay Timestamped wire format `b64Iv:b64Cipher`).
   - `AES_128_CBC`, `CHACHA20` (RFC 8439), `HMAC_SIGNED_ENVELOPE` (SHA-256), and `XOR_ROLLING`.
5. **High-Density Actuator Ecosystem**:
   - **16-Channel Native Relays**: Dynamic custom naming (`iot::on("ExhaustFan")`) & staggered inrush startup.
   - **PCA9685 16-Channel 12-Bit PWM Expander**: Grow light spectrum tuning & fan speed (0-100%) (`iot::dim_pwm(ch, pct)`).
   - **74HC595 / TPIC6B595 Relay Matrix**: Daisy-chain 16, 32, 48, 64+ relays using 3 GPIO pins (`iot::shift_relay(idx, state)`).
   - **Bistable Latching Solenoids**: 50ms pulse open / close with 0mA holding power (`iot::latch_open(pin)`).
   - **Automatic Stepper Feeder**: Anti-jam reverse agitation cycle (`iot::feed_grams(75.0f)`).
6. **Pulse Tipping Bucket Rain Gauge**:
   - Atomic pulse counting, mm/tip calibration, rainfall rate (mm/h), and daily accumulation with midnight auto-reset (`iot::read_rain_rate()`, `iot::read_daily_rain()`).
7. **Bluetooth Serial Monitor & Wireless BLE OTA**:
   - Bluetooth Classic SPP & BLE Nordic UART Service (NUS) serial streaming (`iot::ble_println(...)`).
   - In-field wireless smartphone firmware updating with dual-partition rollback protection.
8. **Tiered Offline Persistence Caching & Auto-Backfill**:
   - Tier-1 RAM Ring Buffer -> Tier-2 Flash Spool -> Tier-3 MicroSD CSV.
   - Automatic backlog playback to cloud servers upon network recovery (`iot::flush_offline_cache()`).
9. **Extensible Custom CLI Engine**:
   - Register custom commands with typed arguments, fluent chaining, and auto-generated help menus (`iot::cli("cmd").description(...).usage(...).on_execute(...)`).
10. **Long-Range LoRa SX1276 & IEEE 802.15.4 Thread Mesh**:
   - 433/868/915 MHz SPI LoRa transceiver for long-range agricultural telemetry + Matter over Thread on ESP32-C6/H2.

---

## 📂 10 Comprehensive Production Examples

Explore complete production-ready examples in [`examples/`](examples/):

- **[`examples/00_minimal_5_lines_quickstart`](examples/00_minimal_5_lines_quickstart/main.cpp)**: 5-line quickstart with auto Wi-Fi and guards.
- **[`examples/01_smart_hydroponics_dosing`](examples/01_smart_hydroponics_dosing/main.cpp)**: NFT/DFT hydroponics, Modbus pH/EC (Nernst & EC25 compensated), daily chemical dosing budget.
- **[`examples/02_precision_agriculture_lora_mesh`](examples/02_precision_agriculture_lora_mesh/main.cpp)**: Solar-powered remote node, 7-in-1 NPK soil probe, LoRa 915 MHz star network, MPU6050 tilt guard, deep sleep (< 15 uA).
- **[`examples/03_industrial_energy_pzem_gateway`](examples/03_industrial_energy_pzem_gateway/main.cpp)**: PZEM AC energy monitoring, MicroSD daily CSV logging, AES-256 encrypted WebSocket.
- **[`examples/04_aquaculture_autofeeder_pool`](examples/04_aquaculture_autofeeder_pool/main.cpp)**: Dissolved Oxygen monitoring, anti-inrush aerator startup, scheduled stepper feeder with anti-jam agitation.
- **[`examples/05_thread_matter_smart_greenhouse`](examples/05_thread_matter_smart_greenhouse/main.cpp)**: ESP32-C6 Wi-Fi 6 + Thread Mesh bridge, PCA9685 spectrum tuning, bistable latching valves.
- **[`examples/06_industrial_weather_station_rain_gauge`](examples/06_industrial_weather_station_rain_gauge/main.cpp)**: Pulse tipping bucket rain gauge, wind speed anemometer, disaster flood SMS alerts.
- **[`examples/07_ble_serial_and_bluetooth_ota`](examples/07_ble_serial_and_bluetooth_ota/main.cpp)**: BLE Nordic UART smartphone CLI terminal + wireless in-field BLE firmware flasher.
- **[`examples/08_multi_servo_matrix_and_sensor_multiplexer`](examples/08_multi_servo_matrix_and_sensor_multiplexer/main.cpp)**: 16-32 Multi-Servo matrix with relay-like behavior, TCA9548A 8-channel I2C MUX, and 1-Wire multi-drop temperature array.
- **[`examples/09_custom_cli_and_advanced_sensors`](examples/09_custom_cli_and_advanced_sensors/main.cpp)**: Custom CLI engine, BME680 IAQ gas, SCD40 NDIR CO2, RTD PT100, and 4-20mA process loop.

---

## 🛠️ Installation & Integration

### PlatformIO
Add to your `platformio.ini`:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev ; or esp32-s3-devkitc-1, esp32-c3-devkitm-1, esp32-c6-devkitc-1
framework = espidf ; or arduino
build_flags = -std=c++20 -O2
lib_deps =
    dhimasardinata/AetherIoT @ ^0.0.1
```

### ESP-IDF Component Registry
Add to your `main/idf_component.yml`:
```yaml
dependencies:
  dhimasardinata/AetherIoT: "^0.0.1"
```

### Arduino IDE
1. Download this repository as `.zip`.
2. Select **Sketch** -> **Include Library** -> **Add .ZIP Library...**.
3. Include `#include "iot_framework.hpp"` in your code.

---

## License

Licensed under the **Apache License, Version 2.0**. See [LICENSE](LICENSE) for details.

Developed with precision by **Dhimas Ardinata** (<dhimasardinatapp@gmail.com>).
