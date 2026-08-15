# AetherIoT: Framework IoT Industri C++20 Terpadu Bebas Alokasi Dinamis (Zero-Heap)

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![PlatformIO Registry](https://img.shields.io/badge/PlatformIO-AetherIoT-orange.svg)](https://registry.platformio.org/)
[![ESP-IDF Component](https://img.shields.io/badge/ESP--IDF-Component-green.svg)](https://components.espressif.com/)
[![Arduino Library](https://img.shields.io/badge/Arduino-Library-00979D.svg)](https://www.arduino.cc/reference/en/libraries/)
[![Zero-Heap](https://img.shields.io/badge/Dynamic%20Memory-0%25%20Heap-brightgreen.svg)]()
[![Documentation](https://img.shields.io/badge/Docs-GitHub%20Pages-blue.svg)](https://dhimasardinata.github.io/AetherIoT/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

**[English](README.md) | [Bahasa Indonesia](README.id.md) | 🌐 [Website Dokumentasi Interaktif](https://dhimasardinata.github.io/AetherIoT/)**

**AetherIoT** adalah framework bare-metal C++20 modern, deterministik, dan sangat andal yang dirancang untuk kebutuhan **IoT Industri, Hidroponik Presisi, Pertanian Pintar, Stasiun Pemantauan Cuaca, Matriks Multi-Servo Kepadatan Tinggi, dan Gateway Telemetri** melintasi **seluruh silikon Espressif** (ESP32 Classic, S2, S3, C2, C3, C6, H2, P4, ESP8266).

---

## ⚡ Panduan Cepat 5 Baris Kode

Mulai aplikasi IoT dalam hitungan detik tanpa kerumitan konfigurasi manual:

```cpp
#include "iot_framework.hpp"

IOT_APP(app) {
    // 1. Pasang Batas Pengaman Otomatis (Safety Guard)
    iot::guard("Suhu").max(35.0f).on_breach([](auto, auto v) {
        iot::log_error("ALERT: Suhu %.1f C melebihi batas aman!", v);
    });

    // 2. Aliran Data Real-Time
    app.on_data([](auto d) {
        iot::log_info("Suhu: %.1f C | Kelembaban: %.1f %%RH | pH: %.2f", d.temp, d.hum, d.ph);
    });
}
```

---

## 🎯 Matriks Target Silikon Espressif yang Didukung Penuh

| Target SoC | Arsitektur CPU & Frekuensi | Fitur Unggulan Hardware yang Dioptimalkan | Rekomendasi Penggunaan Lapangan |
| :--- | :--- | :--- | :--- |
| **ESP32 Classic** | Dual-Core Xtensa LX6 (240MHz) | Wi-Fi 4, BLE 4.2, BT Classic SPP, ULP | Gateway Hidroponik, RS-485 Modbus Hub |
| **ESP32-S3** | Dual-Core Xtensa LX7 (240MHz) | **Instruksi Vektor AI**, Native USB OTG, Octal PSRAM | Edge AI Vision, Deteksi Hama, Fast DSP |
| **ESP32-S2** | Single-Core Xtensa LX7 (240MHz) | Native USB OTG CDC, Deep Sleep Sangat Rendah | Dongle USB Industri, Datalogger Portabel |
| **ESP32-C3** | Single-Core RISC-V 32-bit (160MHz) | Wi-Fi 4, BLE 5.0 Long Range, Hardware Crypto | Node Sensor Pertanian Pintar Efisien |
| **ESP32-C6** | Single-Core RISC-V 32-bit (160MHz) | **Wi-Fi 6**, BLE 5.0, **Thread / Zigbee 802.15.4** | Perkebunan Modern, Bridge Matter Mesh |
| **ESP32-H2** | Single-Core RISC-V 32-bit (96MHz) | **Thread / Zigbee / Matter 802.15.4**, BLE 5.3 | Sensor Nirkabel Matter Daya Ekstrem Rendah |
| **ESP32-P4** | Dual-Core RISC-V Kinerja Tinggi | Antarmuka Layar MIPI/DPI, Hardware H.264 | Panel Layar Sentuh HMI Cerdas Industri |
| **ESP8266** | Single-Core Xtensa L106 (160MHz) | Wi-Fi 4, Telemetri UART Ekonomis | Node Sensor Murah Skala Masif |
| **Host Native** | x86_64 / ARM64 (Linux/macOS/Win) | Emulasi C++20 Lengkap & Mock Bus I2C/SPI | Unit Testing & CI/CD Pipeline Otomatis |

---

## 💎 Fitur & Keunggulan Arsitektur Utama

1. **Jaminan 0% Heap (Zero Dynamic Memory)**:
   - Mencegah fragmentasi memori dan reboot *Out-of-Memory* (OOM) dengan menempatkan seluruh buffer komunikasi, string, queue, dan enkripsi pada stack dan memori statis.
2. **Matriks Multi-Servo Kepadatan Tinggi (Perilaku Mirip Relai)**:
   - Kendalikan hingga 32 servo dengan API deklaratif: `iot::servo_open("KatupUtama")`, `iot::servo_close("KatupUtama")`, `iot::servo_toggle(0)`, `iot::servo_is_open(0)`.
3. **Multiplexing Sensor Kepadatan Tinggi**:
   - **TCA9548A 8-Channel I2C MUX**: Sambungkan banyak sensor I2C dengan alamat kembar (`iot::i2c_channel(ch)`).
   - **1-Wire Multi-Drop Temperature Array**: Baca 16-32 sensor DS18B20 hanya dengan 1 pin GPIO (`iot::read_temp_probe(idx)`).
   - **CD74HC4067 16-Channel Analog MUX / ADS1115 16-Bit ADC**: Perluas input analog (`iot::read_analog_mux_voltage(ch)`).
   - **Modbus Multi-Slave Hub**: Polling hingga 32 sensor Modbus RTU pada satu bus RS-485.
4. **Multi-Scheme Payload Encryption & Anti-Replay**:
   - `AES_256_CBC` (Format `b64Iv:b64Cipher` dengan verifikasi timestamp epoch 4-byte big-endian).
   - `AES_128_CBC`, `CHACHA20` (RFC 8439), `HMAC_SIGNED_ENVELOPE` (SHA-256), dan `XOR_ROLLING`.
5. **Ekosistem Aktuator Kepadatan Tinggi (High-Density Actuators)**:
   - **16-Channel Relai Native**: Penamaan string kustom (`iot::on("KipasExhaust")`) & aktivasi inrush bertahap (staggered).
   - **PCA9685 16-Channel 12-Bit PWM Expander**: Pengaturan spektrum lampu LED tanaman dan kecepatan kipas (0-100%) (`iot::dim_pwm(ch, pct)`).
   - **74HC595 / TPIC6B595 Relay Shift Matrix**: Kendalikan 16, 32, 48, 64+ relai hanya memakai 3 pin GPIO (`iot::shift_relay(idx, state)`).
   - **Bistable Latching Solenoids**: Pulsa 50ms buka/tutup dengan 0mA arus penahan (`iot::latch_open(pin)`).
   - **Automatic Stepper Feeder**: Agitasi bolak-balik anti-gumpalan pakan (`iot::feed_grams(75.0f)`).
6. **Sensor Curah Hujan Tipping Bucket**:
   - Penghitungan pulsa atomik berbasis interupsi, kalibrasi mm/tip, intensitas curah hujan (mm/jam), dan akumulasi harian dengan reset otomatis tengah malam (`iot::read_rain_rate()`, `iot::read_daily_rain()`).
7. **Bluetooth Serial Monitor & BLE OTA Nirkabel**:
   - Terminal Serial via Bluetooth Classic SPP & BLE Nordic UART Service (NUS) ke smartphone tanpa butuh Wi-Fi (`iot::ble_println(...)`).
   - Flashing pembaruan firmware langsung dari smartphone di lapangan dengan proteksi rollback partisi.
8. **Tiered Offline Persistence Caching & Auto-Backfill**:
   - Tier-1 RAM Ring Buffer -> Tier-2 Flash Spool -> Tier-3 MicroSD CSV.
   - Sinkronisasi otomatis data historis ke cloud saat koneksi internet pulih (`iot::flush_offline_cache()`).
9. **Engine CLI Kustom yang Fleksibel**:
   - Daftarkan perintah kustom dengan argumen bertipe data (int, float, bool), chaining fluent, dan menu help otomatis (`iot::cli("cmd").description(...).usage(...).on_execute(...)`).
10. **LoRa SX1276 Jarak Jauh & IEEE 802.15.4 Thread Mesh**:
   - Transceiver LoRa SPI 433/868/915 MHz untuk transmisi kilometer + Matter over Thread pada ESP32-C6/H2.

---

## 📂 10 Contoh Skenario Penggunaan Nyata ([`examples/`](examples/))

Jelajahi berkas contoh siap pakai di direktori [`examples/`](examples/):

- **[`examples/00_minimal_5_lines_quickstart`](examples/00_minimal_5_lines_quickstart/main.cpp)**: Contoh 5 baris super ringkas dengan auto Wi-Fi dan safety guards.
- **[`examples/01_smart_hydroponics_dosing`](examples/01_smart_hydroponics_dosing/main.cpp)**: Hidroponik NFT/DFT, Modbus pH/EC terkompensasi suhu Nernst & EC25, kuota dosis kimia harian.
- **[`examples/02_precision_agriculture_lora_mesh`](examples/02_precision_agriculture_lora_mesh/main.cpp)**: Tiang sensor tenaga surya, probe tanah 7-in-1 NPK, LoRa 915 MHz, guard kemiringan tiang MPU6050, deep sleep (< 15 uA).
- **[`examples/03_industrial_energy_pzem_gateway`](examples/03_industrial_energy_pzem_gateway/main.cpp)**: Pengukur daya AC PZEM-004T, log CSV harian MicroSD FAT32, WebSocket terenkripsi AES-256.
- **[`examples/04_aquaculture_autofeeder_pool`](examples/04_aquaculture_autofeeder_pool/main.cpp)**: Pemantau Oksigen Terlarut (DO) tambak, aerator bertahap, feeder pakan otomatis anti-gumpalan.
- **[`examples/05_thread_matter_smart_greenhouse`](examples/05_thread_matter_smart_greenhouse/main.cpp)**: ESP32-C6 Wi-Fi 6 + Thread Mesh bridge, PCA9685 spectrum tuning lampu tanaman, katup irigasi latching solenoid.
- **[`examples/06_industrial_weather_station_rain_gauge`](examples/06_industrial_weather_station_rain_gauge/main.cpp)**: Stasiun cuaca curah hujan tipping bucket, sensor angin, SMS peringatan dini banjir.
- **[`examples/07_ble_serial_and_bluetooth_ota`](examples/07_ble_serial_and_bluetooth_ota/main.cpp)**: Terminal CLI Bluetooth HP teknisi + flashing firmware OTA nirkabel via BLE di lapangan.
- **[`examples/08_multi_servo_matrix_and_sensor_multiplexer`](examples/08_multi_servo_matrix_and_sensor_multiplexer/main.cpp)**: Matriks 16-32 servo mirip relai, multiplexer I2C 8-channel TCA9548A, dan array suhu 1-Wire.
- **[`examples/09_custom_cli_and_advanced_sensors`](examples/09_custom_cli_and_advanced_sensors/main.cpp)**: Engine Custom CLI kustom, sensor gas BME680 IAQ, CO2 SCD40 NDIR, RTD PT100, dan pemancar arus analog 4-20mA.

---

## 🛠️ Panduan Instalasi & Registri

### PlatformIO
Tambahkan pada file `platformio.ini`:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev ; atau esp32-s3-devkitc-1, esp32-c3-devkitm-1, esp32-c6-devkitc-1
framework = espidf ; atau arduino
build_flags = -std=c++20 -O2
lib_deps =
    dhimasardinata/AetherIoT @ ^0.0.1
```

### ESP-IDF Component Registry
Tambahkan pada file `main/idf_component.yml`:
```yaml
dependencies:
  dhimasardinata/AetherIoT: "^0.0.1"
```

### Arduino IDE
1. Unduh repositori ini sebagai file `.zip`.
2. Buka menu **Sketch** -> **Include Library** -> **Add .ZIP Library...**.
3. Panggil `#include "iot_framework.hpp"` pada proyek Anda.

---

## Lisensi

Dilisensikan di bawah **Apache License, Version 2.0**. Lihat berkas [LICENSE](LICENSE) untuk rincian lengkap.

Dikembangkan secara presisi oleh **Dhimas Ardinata** (<dhimasardinatapp@gmail.com>).
