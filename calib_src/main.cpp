// Weight Stack Pin Position Calibration Tool
// Build:  pio run -e calib -t upload
// Use:    pio device monitor
//
// Workflow:
//   1. Open Serial Monitor (115200 baud)
//   2. Press 's' + Enter  — start
//   3. Insert pin in position 1 (5 kg)
//   4. Press Enter        — takes 25 measurements, shows stats
//   5. Repeat for all 17 positions
//   6. Full table printed at the end

#include <Arduino.h>
#include <Wire.h>

// ─── Config ───────────────────────────────────────────────
static constexpr uint8_t  PIN_SDA       = 21;
static constexpr uint8_t  PIN_SCL       = 22;
static constexpr uint8_t  PIN_XSHUT     = 16;
static constexpr uint8_t  ADDR          = 0x29;
static constexpr uint8_t  NUM_POS       = 17;
static constexpr uint8_t  FIRST_KG      = 5;
static constexpr uint8_t  STEP_KG       = 5;
static constexpr uint8_t  SAMPLES       = 25;
static constexpr uint16_t JITTER_WARN   = 30;   // mm
static constexpr uint16_t DELTA_WARN    = 15;   // mm — adjacent positions too close

// ─── VL53L0X register I/O (exact same as production driver) ──

static uint8_t g_stopVar = 0;

static uint8_t readReg(uint8_t reg) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(ADDR, (uint8_t)1);
    uint8_t v = Wire.read();
    delayMicroseconds(500);
    return v;
}

static void writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission(true);
    delayMicroseconds(500);
}

static uint16_t readReg16(uint8_t reg) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(ADDR, (uint8_t)2);
    uint16_t v = (uint16_t)Wire.read() << 8;
    v |= Wire.read();
    delayMicroseconds(500);
    return v;
}

static void writeReg16(uint8_t reg, uint16_t val) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.write((val >> 8) & 0xFF);
    Wire.write(val & 0xFF);
    Wire.endTransmission(true);
    delayMicroseconds(500);
}

static bool doSingleRef(uint8_t vhvInitByte) {
    writeReg(0x00, 0x01 | vhvInitByte);
    delay(10);
    uint32_t t0 = millis();
    while ((readReg(0x13) & 0x07) == 0) {
        if (millis() - t0 > 1000) return false;
        delay(10);
    }
    writeReg(0x0B, 0x01);
    writeReg(0x00, 0x00);
    return true;
}

static bool initSensor() {
    if (readReg(0xC0) != 0xEE) return false;
    writeReg(0x89, readReg(0x89) | 0x01);
    writeReg(0x88, 0x00);
    writeReg(0x80, 0x01); writeReg(0xFF, 0x01); writeReg(0x00, 0x00);
    g_stopVar = readReg(0x91);
    writeReg(0x00, 0x01); writeReg(0xFF, 0x00); writeReg(0x80, 0x00);
    writeReg(0x60, readReg(0x60) | 0x12);
    writeReg16(0x44, 32);
    writeReg(0x01, 0xE8);
    writeReg(0x0A, 0x04);
    writeReg(0x84, (readReg(0x84) & ~0x10) | 0x10);
    writeReg(0x0B, 0x01);
    writeReg(0x01, 0x01);
    if (!doSingleRef(0x40)) return false;
    writeReg(0x01, 0x02);
    if (!doSingleRef(0x00)) return false;
    writeReg(0x01, 0xE8);
    return true;
}

static uint16_t measure() {
    writeReg(0x80, 0x01); writeReg(0xFF, 0x01); writeReg(0x00, 0x00);
    writeReg(0x91, g_stopVar);
    writeReg(0x00, 0x01); writeReg(0xFF, 0x00); writeReg(0x80, 0x00);
    delay(5);
    writeReg(0x00, 0x01);
    uint32_t t0 = millis();
    while (readReg(0x00) & 0x01) {
        if (millis() - t0 > 500) return 0xFFFF;
        delay(5);
    }
    t0 = millis();
    while ((readReg(0x13) & 0x07) == 0) {
        if (millis() - t0 > 500) return 0xFFFF;
        delay(5);
    }
    uint16_t d = readReg16(0x14 + 0x0A);
    writeReg(0x0B, 0x01);
    return d;
}

// ─── Calibration state ────────────────────────────────────

struct CalPoint {
    uint16_t dist, dmin, dmax, jitter;
    bool valid;
};

static CalPoint table[NUM_POS];
static uint8_t  currentPos  = 0;
static bool     calibrating = false;
static bool     liveMode    = false;

static uint8_t posKg(uint8_t i) { return FIRST_KG + i * STEP_KG; }

// ─── Output helpers ───────────────────────────────────────

static void printSep() {
    Serial.println("-----+-----+---------+------+------+--------+--------");
}

static void printHelp() {
    Serial.println();
    Serial.println("========================================");
    Serial.println("  WEIGHT STACK CALIBRATION  v1.0");
    Serial.println("========================================");
    Serial.println("  s     - start calibration");
    Serial.println("  ENTER - confirm / take measurement");
    Serial.println("  r     - repeat current position");
    Serial.println("  p     - print table so far");
    Serial.println("  l     - toggle live distance mode");
    Serial.println("  x     - reset all data");
    Serial.println("  h     - this help");
    Serial.println("========================================");
    Serial.println();
}

static void printTable() {
    Serial.println();
    Serial.println("   #  | kg  | dist_mm | min  | max  | jitter |  delta");
    printSep();

    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_POS; i++) {
        if (!table[i].valid) continue;
        count++;

        bool    hasDelta = (i > 0 && table[i - 1].valid);
        int16_t delta    = hasDelta
            ? (int16_t)table[i].dist - (int16_t)table[i - 1].dist
            : 0;

        char buf[72];
        if (hasDelta)
            snprintf(buf, sizeof(buf), "  %2d  | %3d | %7d | %4d | %4d | %6d | %+6d",
                i + 1, posKg(i),
                table[i].dist, table[i].dmin, table[i].dmax, table[i].jitter,
                delta);
        else
            snprintf(buf, sizeof(buf), "  %2d  | %3d | %7d | %4d | %4d | %6d |    ---",
                i + 1, posKg(i),
                table[i].dist, table[i].dmin, table[i].dmax, table[i].jitter);

        Serial.print(buf);
        if (table[i].jitter > JITTER_WARN)                Serial.print("  !! JITTER");
        if (hasDelta && abs(delta) < (int16_t)DELTA_WARN) Serial.print("  !! TOO CLOSE");
        Serial.println();
    }

    printSep();
    if (count == 0) { Serial.println("  No data yet."); Serial.println(); return; }

    bool clean = true;
    for (uint8_t i = 0; i < NUM_POS; i++) {
        if (!table[i].valid) continue;
        if (table[i].jitter > JITTER_WARN) {
            Serial.printf("  WARN pos %d (%d kg): jitter=%d mm, re-measure recommended\n",
                i + 1, posKg(i), table[i].jitter);
            clean = false;
        }
        if (i > 0 && table[i - 1].valid) {
            uint16_t gap = (uint16_t)abs((int16_t)table[i].dist - (int16_t)table[i - 1].dist);
            if (gap < DELTA_WARN) {
                Serial.printf("  WARN pos %d/%d: only %d mm apart, sensor may confuse them\n",
                    i, i + 1, gap);
                clean = false;
            }
        }
    }
    if (clean) Serial.println("  All positions look OK.");
    Serial.println();
}

static void promptPos() {
    if (currentPos >= NUM_POS) {
        Serial.println();
        Serial.println("=== CALIBRATION COMPLETE ===");
        printTable();
        calibrating = false;
        return;
    }
    Serial.println();
    Serial.printf("--- [%d / %d]  Insert pin -> position %d  (%d kg) ---\n",
        currentPos + 1, NUM_POS, currentPos + 1, posKg(currentPos));
    Serial.println("    Keep pin still, then press ENTER on your computer.");
}

// ─── Core measurement step ────────────────────────────────

static bool doMeasure(uint8_t idx) {
    uint16_t buf[SAMPLES];
    uint8_t  valid = 0;

    Serial.print("  Sampling");
    for (uint8_t i = 0; i < SAMPLES; i++) {
        uint16_t d = measure();
        if (d > 0 && d < 8190) buf[valid++] = d;
        Serial.print('.');
        delay(50);
    }
    Serial.println();

    if (valid < SAMPLES / 2) {
        Serial.println("  ERROR: too many bad readings. Check sensor and try again.");
        return false;
    }

    // Simple insertion sort for median / min / max
    for (uint8_t i = 1; i < valid; i++) {
        uint16_t key = buf[i];
        int8_t j = (int8_t)i - 1;
        while (j >= 0 && buf[j] > key) { buf[j + 1] = buf[j]; j--; }
        buf[j + 1] = key;
    }

    uint16_t mn     = buf[0];
    uint16_t mx     = buf[valid - 1];
    uint16_t median = buf[valid / 2];
    uint16_t jitter = mx - mn;

    table[idx] = { median, mn, mx, jitter, true };

    Serial.printf("  Median : %d mm\n", median);
    Serial.printf("  Min    : %d mm\n", mn);
    Serial.printf("  Max    : %d mm\n", mx);
    Serial.printf("  Jitter : %d mm%s\n",
        jitter, jitter > JITTER_WARN ? "  !! HIGH — was pin fully inserted?" : "");

    return true;
}

static void resetAll() {
    for (uint8_t i = 0; i < NUM_POS; i++) table[i].valid = false;
    currentPos  = 0;
    calibrating = false;
    liveMode    = false;
    Serial.println("Reset. All calibration data cleared.");
}

// ─── Arduino entry points ─────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(2000);

    pinMode(PIN_XSHUT, OUTPUT);
    digitalWrite(PIN_XSHUT, LOW);
    delay(200);
    digitalWrite(PIN_XSHUT, HIGH);
    delay(500);

    Wire.begin();
    Wire.setClock(100000);

    Serial.println("\n\nInitializing VL53L0X...");
    if (!initSensor()) {
        Serial.println("ERROR: sensor not found or init failed! Check wiring.");
        while (1) delay(1000);
    }
    Serial.println("Sensor OK.");

    printHelp();
    Serial.println("Ready. Type 's' + Enter to start, or 'l' for live distance.");
}

void loop() {
    // ── Live mode: print distance every 200 ms, stop on 'l' ──
    if (liveMode) {
        uint16_t d = measure();
        if (d > 0 && d < 8190) Serial.printf("LIVE: %4d mm\n", d);
        else                    Serial.println("LIVE:  ---");
        delay(200);
        while (Serial.available()) {
            char c = (char)Serial.read();
            if (c == 'l' || c == 'L') {
                liveMode = false;
                Serial.println("Live mode OFF.");
            }
        }
        return;
    }

    if (!Serial.available()) return;

    String line = Serial.readStringUntil('\n');
    line.trim();
    char cmd = line.length() > 0 ? (char)tolower((unsigned char)line[0]) : '\0';

    switch (cmd) {

        case 'h':
            printHelp();
            break;

        case 'l':
            liveMode = true;
            Serial.println("Live mode ON. Send 'l' to stop.");
            break;

        case 'x':
            resetAll();
            break;

        case 'p':
            printTable();
            break;

        case 's':
            if (!calibrating) {
                calibrating = true;
                currentPos  = 0;
                Serial.println("Starting calibration...");
                promptPos();
            } else {
                Serial.println("Already in progress. ENTER=measure  r=repeat  x=reset");
            }
            break;

        case 'r':
            if (calibrating) { Serial.println("Repeating position..."); promptPos(); }
            else              { Serial.println("Not calibrating. Press 's' to start."); }
            break;

        case '\0':  // bare ENTER key
            if (calibrating) {
                if (doMeasure(currentPos)) {
                    currentPos++;
                    promptPos();
                } else {
                    Serial.println("Press ENTER to retry, or 'r' to re-read prompt.");
                }
            } else {
                Serial.println("Not calibrating. Press 's' to start.");
            }
            break;

        default:
            Serial.printf("Unknown: '%c'. Type 'h' for help.\n", cmd);
            break;
    }
}
