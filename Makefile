# ============================================================================
# AetherIoT High-Performance Cached Build System
# ============================================================================
# Usage:
#   make -j$(nproc)       # Build all 11 targets in parallel with build cache
#   make clean            # Remove build artifacts
# ============================================================================

CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -O2 -I.
BUILD_DIR ?= .build_cache

HEADERS := $(wildcard *.hpp)

TARGETS := $(BUILD_DIR)/main_app \
           $(BUILD_DIR)/ex00_quickstart \
           $(BUILD_DIR)/ex01_hydroponics \
           $(BUILD_DIR)/ex02_agri_lora \
           $(BUILD_DIR)/ex03_energy_pzem \
           $(BUILD_DIR)/ex04_aquaculture \
           $(BUILD_DIR)/ex05_thread_matter \
           $(BUILD_DIR)/ex06_weather_station \
           $(BUILD_DIR)/ex07_ble_ota \
           $(BUILD_DIR)/ex08_multi_servo \
           $(BUILD_DIR)/ex09_custom_cli

all: $(TARGETS)
	@echo "\033[1;32m[BUILD SUCCESS] All 11 targets compiled cleanly!\033[0m"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/main_app: main.cpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling main..."
	@$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/ex00_quickstart: examples/00_minimal_5_lines_quickstart/main.cpp examples/00_minimal_5_lines_quickstart/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 00 (Quickstart)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/00_minimal_5_lines_quickstart $< -o $@

$(BUILD_DIR)/ex01_hydroponics: examples/01_smart_hydroponics_dosing/main.cpp examples/01_smart_hydroponics_dosing/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 01 (Hydroponics)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/01_smart_hydroponics_dosing $< -o $@

$(BUILD_DIR)/ex02_agri_lora: examples/02_precision_agriculture_lora_mesh/main.cpp examples/02_precision_agriculture_lora_mesh/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 02 (Agri LoRa)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/02_precision_agriculture_lora_mesh $< -o $@

$(BUILD_DIR)/ex03_energy_pzem: examples/03_industrial_energy_pzem_gateway/main.cpp examples/03_industrial_energy_pzem_gateway/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 03 (Energy PZEM)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/03_industrial_energy_pzem_gateway $< -o $@

$(BUILD_DIR)/ex04_aquaculture: examples/04_aquaculture_autofeeder_pool/main.cpp examples/04_aquaculture_autofeeder_pool/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 04 (Aquaculture)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/04_aquaculture_autofeeder_pool $< -o $@

$(BUILD_DIR)/ex05_thread_matter: examples/05_thread_matter_smart_greenhouse/main.cpp examples/05_thread_matter_smart_greenhouse/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 05 (Thread Matter)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/05_thread_matter_smart_greenhouse $< -o $@

$(BUILD_DIR)/ex06_weather_station: examples/06_industrial_weather_station_rain_gauge/main.cpp examples/06_industrial_weather_station_rain_gauge/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 06 (Weather Station)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/06_industrial_weather_station_rain_gauge $< -o $@

$(BUILD_DIR)/ex07_ble_ota: examples/07_ble_serial_and_bluetooth_ota/main.cpp examples/07_ble_serial_and_bluetooth_ota/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 07 (BLE OTA)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/07_ble_serial_and_bluetooth_ota $< -o $@

$(BUILD_DIR)/ex08_multi_servo: examples/08_multi_servo_matrix_and_sensor_multiplexer/main.cpp examples/08_multi_servo_matrix_and_sensor_multiplexer/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 08 (Multi-Servo)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/08_multi_servo_matrix_and_sensor_multiplexer $< -o $@

$(BUILD_DIR)/ex09_custom_cli: examples/09_custom_cli_and_advanced_sensors/main.cpp examples/09_custom_cli_and_advanced_sensors/config.hpp $(HEADERS) | $(BUILD_DIR)
	@echo "[CXX] Compiling Example 09 (Custom CLI)..."
	@$(CXX) $(CXXFLAGS) -Iexamples/09_custom_cli_and_advanced_sensors $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
	@echo "[CLEAN] Build cache removed."

.PHONY: all clean
