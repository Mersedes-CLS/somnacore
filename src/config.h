#pragma once

#include <cstdint>

// --- Pin assignments ---
constexpr uint8_t PIN_SDA        = 21;
constexpr uint8_t PIN_SCL        = 22;
constexpr uint8_t PIN_XSHUT_TOP  = 16;

// --- I2C ---
constexpr uint8_t  ADDR_VL53L0X  = 0x29;
constexpr uint32_t I2C_CLOCK_HZ  = 100000;

// --- Median filter ---
constexpr uint8_t  FILTER_SIZE   = 3;
constexpr uint16_t DIST_MIN      = 30;
constexpr uint16_t DIST_MAX      = 2000;

// --- Timing ---
constexpr uint16_t SENSOR_BOOT_MS     = 500;
constexpr uint16_t XSHUT_LOW_MS       = 200;
constexpr uint16_t MEAS_TIMEOUT_MS    = 500;
constexpr uint16_t CAL_TIMEOUT_MS     = 1000;
constexpr uint8_t  MAX_SENSOR_ERRORS  = 3;
constexpr uint8_t  MAX_INIT_RETRIES   = 5;
constexpr uint16_t LOOP_DELAY_MS      = 50;

// --- WiFi ---
constexpr const char* WIFI_SSID = "TP-Link_1018";
constexpr const char* WIFI_PASS = "46657763";
constexpr uint8_t  WIFI_MAX_ATTEMPTS  = 20;
constexpr uint16_t WIFI_RETRY_MS      = 500;

// --- Rep detection ---
constexpr float    REP_ENTER_PCT      = 0.30f;
constexpr float    REP_EXIT_PCT       = 0.10f;
constexpr uint16_t REP_MIN_MS         = 300;
constexpr uint8_t  REP_STABLE_COUNT   = 3;
constexpr float    BASELINE_DECAY     = 0.98f;
constexpr float    BASELINE_ADAPT     = 0.02f;
constexpr uint32_t SET_TIMEOUT_MS     = 30000;
constexpr uint8_t  MAX_SETS           = 32;

// --- Web server ---
constexpr uint16_t HTTP_PORT = 80;

// --- Machine identity ---
constexpr const char* MACHINE_ID = "machine_01";

// --- Backend server ---
constexpr const char* BACKEND_BASE = "https://somnacore-production.up.railway.app";
constexpr const char* BACKEND_URL  = "https://somnacore-production.up.railway.app/api/set";

// --- Calibration remote timing ---
constexpr uint16_t CALIB_PUSH_INTERVAL_MS = 3000;
constexpr uint16_t CALIB_POLL_INTERVAL_MS = 3000;
