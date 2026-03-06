# SomnaCore Hardware

## 1. Distance Sensor

**VL53L0X (GY-530) Time-of-Flight Laser Distance Sensor**

Interface: I2C\
Supply Voltage: 2.8 -- 5 V\
Logic Level: 3.3 V\
Current Consumption: \~19 mA\
Wavelength: 940 nm (VCSEL)\
Measurement Range: up to 2 m\
Operating Temperature: −20°C to +70°C

Technology: - VCSEL laser emitter - SPAD photon detector array -
Time-of-Flight distance measurement

Usage in SomnaCore: - Top sensor → repetition detection - Bottom sensor
→ stack plate position (weight detection)

------------------------------------------------------------------------

## 2. Microcontroller

**ESP32 NodeMCU DEVKIT V1 (ESP32-WROOM, CP2102)**

CPU: Dual-core Xtensa LX6 up to 240 MHz\
Wi‑Fi: 802.11 b/g/n (2.4 GHz)\
Bluetooth: BLE v4.2 + BR/EDR

Memory: - SRAM: 520 KB - External Flash: 4 MB

GPIO: - \~25 GPIO pins - 15 ADC inputs (12-bit) - 2 DAC outputs
(8-bit) - PWM: 16 channels (up to 16-bit)

Interfaces: - 3× SPI - 3× UART - 2× I2C - 2× I2S

Logic Voltage: 3.3 V

Board Power: - VIN: 5--14 V - 3V3 output: up to \~1 A

------------------------------------------------------------------------

## 3. NFC / RFID Reader

**PN532 NFC/RFID Module (13.56 MHz)**

Chip: NXP PN532

Supported Standards: - ISO14443 Type A - ISO14443 Type B

Interfaces: - SPI - I2C - UART

Supply Voltage: 3.3 -- 5.5 V\
Logic Level: 3.3 V

Current Consumption: - Active: up to 150 mA - Idle: \~100 mA

Communication Distance: \~5 cm

Interface Selection:

  SET0   SET1   Mode
  ------ ------ ------
  L      L      UART
  L      H      SPI
  H      L      I2C

Usage in SomnaCore: - SomnaBand (NFC bracelet) identification - User
authentication - Training session linking

------------------------------------------------------------------------

## 4. Display

**OLED Display 0.96" 128×64 (I2C)**

Resolution: 128 × 64\
Interface: I2C\
Supply Voltage: 3 -- 5 V\
Power Consumption: \~0.08 W

Viewing Angle: \~160°\
Operating Temperature: −30°C to +70°C

Usage: - display repetitions - display weight - display user info

------------------------------------------------------------------------

## 5. NFC Tags

**NTAG213 NFC Tag**

Frequency: 13.56 MHz\
Standard: NFC Forum Type 2\
Memory: 144 bytes usable

Usage: - SomnaBand user bracelet - user identification

------------------------------------------------------------------------

## 6. Prototyping

**Breadboard**

Type: solderless\
Capacity: 400 tie points

Usage: - prototyping circuit - testing sensors and wiring

------------------------------------------------------------------------

## Current SomnaCore Hardware Topology

ESP32 (SomnaNode)\
├── VL53L0X (Top) → repetitions\
├── VL53L0X (Bottom) → weight detection\
├── PN532 → NFC user identification (SomnaBand)\
└── OLED 128×64 → display

Communication bus: - I2C → sensors + display
