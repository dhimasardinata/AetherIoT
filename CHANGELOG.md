# Changelog

All notable changes to the **AetherIoT** framework will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.0.1] - 2026-08-15

### Added
- **Initial Release of AetherIoT Framework**: High-performance, zero-heap deterministic C++20 bare-metal industrial IoT framework for all Espressif silicon targets (ESP32 Classic, S2, S3, C2, C3, C6, H2, P4, ESP8266).
- **Extensible Custom CLI Engine (`iot_cli.hpp`)**:
  - Declarative custom command registration (`iot::cli("name").description(...).usage(...).on_execute(...)`).
  - Automatic typed argument parsing (`ctx.arg()`, `ctx.arg_int()`, `ctx.arg_float()`, `ctx.arg_bool()`).
  - Multi-channel response dispatch (`ctx.respond_ok()`, `ctx.respond_error()`) and automatic `help` menu generator.
- **Advanced Environmental & Industrial Sensors (`iot_environmental_advanced.hpp`)**:
  - BME680 Gas Resistance & Indoor Air Quality (IAQ Index 0-500).
  - SCD30 / SCD40 True Optical NDIR CO2 Sensor (0 - 40,000 PPM).
  - MAX31865 RTD PT100 / PT1000 Platinum Resistance Interface with Callendar-Van Dusen linearization.
  - JSN-SR04T Waterproof Ultrasonic Level Sensor with temperature compensation.
  - SCT-013 Split-Core Current Transformer True-RMS AC Ammeter.
- **Precision Stepper Motor Controller (`iot_stepper_motor.hpp`)**:
  - Compatible with A4988, DRV8825, and TMC2208/TMC2209 SilentStepStick.
  - Microstepping mode selection (1/1 to 1/32) and anti-jam reverse agitation cycle.
- **Industrial 4-20mA & 0-10V Analog Transmitter (`iot_dac_loop.hpp`)**:
  - Calibrated 12-bit DAC output for industrial process loop valves and VFD inverters.
- **High-Density Multi-Servo Matrix (`iot_multi_servo.hpp`)**:
  - Up to 32 independent servos with declarative relay-like API (`servo_open()`, `servo_close()`, `servo_toggle()`, `servo_name()`, `servo_is_open()`).
- **High-Density Sensor Multiplexing (`iot_sensor_multiplexer.hpp`)**:
  - TCA9548A 8-Channel I2C Bus Multiplexer.
  - 1-Wire DS18B20 16-Drop Multi-Temperature Array on a single GPIO.
  - CD74HC4067 16-Channel Analog Sensor Multiplexer.
  - Modbus RS-485 Multi-Slave Polling Hub (up to 32 slaves).
- **Decoupled Security & Anti-Replay Architecture (`iot_security.hpp`)**:
  - Independent symmetric ciphers: `AES_256_CBC`, `AES_128_CBC`, `CHACHA20`, `XOR_STREAM`, `HMAC_SHA256`.
  - Orthogonal modular Anti-Replay protection layer (Timestamp windowing & Monotonic counters).
- **Tipping Bucket Rain Gauge (`iot_rain_gauge.hpp`)**: Atomic pulse counting, mm/tip calibration, rainfall rate (mm/h), and daily accumulation with midnight auto-reset.
- **Bluetooth Serial Monitor & BLE OTA Flasher (`iot_ble_serial_ota.hpp`)**: Bluetooth Classic SPP & BLE Nordic UART Service (NUS) terminal streaming and in-field wireless smartphone firmware updating.
- **Tiered Persistence Offline Caching & Backfill (`iot_caching_tiered.hpp`)**: Tier-1 RAM Ring Buffer -> Tier-2 Flash Spool -> Tier-3 MicroSD CSV with automatic server backfill on reconnect.
- **High-Density Actuator Expanders**: PCA9685 16-Ch 12-Bit PWM, 74HC595 16-64 Relay Matrix, Bistable Latching Solenoids.
- **Long-Range LoRa SX1276 & IEEE 802.15.4 Thread / Matter Mesh Radio**.
- **Interactive Multi-Language Web Documentation Portal (`docs/`)**.
- **10 Comprehensive Production Examples in `examples/`**:
  - `00_minimal_5_lines_quickstart`: 5-line declarative entry point.
  - `01_smart_hydroponics_dosing`: Precision NFT/DFT dosing with Nernst & EC25 compensation.
  - `02_precision_agriculture_lora_mesh`: Solar-powered remote node, 7-in-1 NPK, LoRa 915 MHz, MPU6050 tilt guard, deep sleep (< 15 uA).
  - `03_industrial_energy_pzem_gateway`: PZEM AC energy monitoring, MicroSD daily CSV logging, AES-256 encrypted WebSocket.
  - `04_aquaculture_autofeeder_pool`: Dissolved Oxygen monitoring, aerator automation, scheduled stepper feeder with anti-jam agitation.
  - `05_thread_matter_smart_greenhouse`: ESP32-C6 Wi-Fi 6 + Thread Mesh bridge, PCA9685 spectrum tuning, bistable latching valves.
  - `06_industrial_weather_station_rain_gauge`: Pulse tipping bucket rain gauge, wind speed anemometer, flood SMS alerts.
  - `07_ble_serial_and_bluetooth_ota`: BLE Nordic UART smartphone CLI terminal + wireless in-field BLE firmware flasher.
  - `08_multi_servo_matrix_and_sensor_multiplexer`: 16-32 Multi-Servo matrix with relay-like behavior, TCA9548A 8-channel I2C MUX, and 1-Wire multi-drop temperature array.
  - `09_custom_cli_and_advanced_sensors`: Custom extensible CLI engine, BME680 IAQ gas, SCD40 NDIR CO2, RTD PT100, and 4-20mA loop transmitter.
