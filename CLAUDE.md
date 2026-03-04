# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino sketch for ESP32 + VL53L0X (GY-530) laser distance sensor. Measures distance in mm via I2C and outputs to Serial Monitor.

## Hardware

- **MCU**: ESP32 NodeMCU 30-pin DevKit V1 (ESP32-D0WD-V3)
- **Sensor**: VL53L0X (GY-530 module) — Time-of-Flight laser ranging, I2C address 0x29
- **Note**: This sensor reports revision 1.15 — Adafruit_VL53L0X library rejects it ("expected cut 1.1 but found 1,15"). Use **Pololu VL53L0X** library instead.

## Wiring

| GY-530 | ESP32          |
|--------|----------------|
| VIN    | VIN (5V)       |
| GND    | GND            |
| SCL    | GPIO22 (SCL)   |
| SDA    | GPIO21 (SDA)   |
| XSHUT  | GPIO16         |

Power via VIN (5V), not 3.3V — sensor was not detected on 3.3V with this board.
XSHUT wire prevents floating XSHUT from putting sensor into standby. Alternative: solder XSHUT to VCC on the GY-530 board.

## Build & Upload (Arduino IDE 2.x)

- Board: **ESP32 Dev Module**
- Upload Speed: 115200 (lower is more reliable for this board)
- Serial Monitor: **115200 baud**
- **Disconnect sensor wires before uploading** — they interfere with flash programming
- Upload requires manual boot mode: press Upload → when `Connecting...` appears, hold BOOT + tap EN + release BOOT

## Libraries

- **VL53L0X by Pololu** (1.3.1) — use this, not Adafruit
- Wire.h (built-in)

## Known Issues

- ESP32 auto-reset for upload does not work reliably — manual BOOT+EN sequence required
- `Wire.begin(21, 22)` with explicit pins can cause boot loops — use `Wire.begin()` without arguments (defaults to GPIO21/22 on ESP32)
- Adafruit_VL53L0X library incompatible with this sensor revision (1.15)

## Debugging Approach — Step-by-Step Sketches

Isolated test sketches in `step_A` through `step_E` subdirectories. Upload each one separately, verify output, then move to next.

| Step | Folder | Purpose | Pass Criteria |
|------|--------|---------|---------------|
| A | `step_A_i2c_scan/` | I2C bus scan | `0x29` found |
| B | `step_B_read_id/` | Read ID registers | `0xC0` = `0xEE` |
| C | `step_C_write_read/` | Write/read reg 0x80 | Read-back matches write |
| D | `step_D_minimal_init/` | Full init sequence | VHV + Phase cal OK |
| E | `step_E_single_shot/` | Single-shot ranging | Distance 50–2000 mm |

## Key VL53L0X Registers

| Register | Name | Expected / Usage |
|----------|------|-----------------|
| 0xC0 | Model ID | 0xEE |
| 0xC1 | Module Type | 0xAA |
| 0xC2 | Revision | varies (0x10 = rev 1.0) |
| 0x80 | Gate (scratch) | Used in init preamble (write 0x01/0x00) |
| 0x89 | VHV config | bit 0 = 2V8 mode |
| 0x88 | I2C mode | 0x00 = standard mode |
| 0x91 | Stop variable | Read during init, used before each measurement |
| 0x60 | MSRC config | OR with 0x12 to disable limit checks |
| 0x44 | Signal rate limit | 32 = 0.25 MCPS (fixed point 9.7) |
| 0x01 | Sequence config | 0xE8 = DSS+VHV+PhaseCal+FinalRange |
| 0x0A | GPIO function | 0x04 = new sample ready |
| 0x00 | SYSRANGE_START | 0x01 = start, bit 0 clears when accepted |
| 0x13 | Interrupt status | bits [2:0] != 0 = measurement ready |
| 0x0B | Interrupt clear | Write 0x01 to clear |
| 0x1E | Range result | 16-bit distance in mm (reg 0x14+0x0A) |
| 0x84 | GPIO HV MUX | bit 4 = active polarity |

## Register-Level Init Sequence

1. Verify Model ID (0xC0 == 0xEE)
2. Set 2V8 mode (0x89 |= 0x01)
3. I2C standard mode (0x88 = 0x00)
4. Read stop_variable (0x80→0xFF→0x00→read 0x91→0x00→0xFF→0x80)
5. Disable MSRC/TCC (0x60 |= 0x12)
6. Signal rate limit (0x44 = 32)
7. Sequence config (0x01 = 0xE8)
8. GPIO interrupt config (0x0A = 0x04, 0x84 polarity, 0x0B clear)
9. VHV calibration (set 0x01=0x01, start with 0x40, wait 0x13)
10. Phase calibration (set 0x01=0x02, start with 0x00, wait 0x13)
11. Restore sequence config (0x01 = 0xE8)

## Common I2C Failure Modes

| Symptom | Likely Cause |
|---------|-------------|
| I2C scan finds 0 devices | GND not connected, SDA/SCL swapped, wrong voltage (use VIN 5V), XSHUT floating low |
| All registers read 0xFF | SDA line issue — bad contact, missing pullup, wrong pin |
| All registers read 0x00 | SDA stuck low — bus recovery needed, or sensor hung |
| `endTransmission()` returns 2 | NACK on address — sensor not responding (power/XSHUT issue) |
| `endTransmission()` returns 3 | NACK on data — register address rejected |
| Boot loop on startup | `Wire.begin(21,22)` bug — use `Wire.begin()` without args |
| VHV/Phase cal timeout | Init steps 1–8 failed silently, or sensor not properly reset via XSHUT |
| Distance = 0 or 8190 | Object out of range, no reflective surface, or laser blocked |
| Measurement timeout (0x13=0) | Sensor didn't start ranging — check sequence config (0x01 should be 0xE8) |
