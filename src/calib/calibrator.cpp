#include "calibrator.h"
#include "../config.h"
#include "../drivers/vl53l0x.h"
#include "../net/web_server.h"
#include <Arduino.h>

namespace calib {

void Calibrator::begin(VL53L0X* sensor) {
    sensor_ = sensor;
    resetAll();
    Serial.println();
    Serial.println("--- Serial calibration ready ---");
    Serial.println("  h + Enter  for help");
    Serial.println("--------------------------------");
}

// ─── Output helpers ───────────────────────────────────────

void Calibrator::printHelp() {
    Serial.println();
    Serial.println("========================================");
    Serial.println("  WEIGHT STACK CALIBRATION  v1.0");
    Serial.println("  Web UI still works in parallel.");
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

static void printSep() {
    Serial.println("-----+-----+---------+------+------+--------+--------");
}

void Calibrator::printTable() {
    Serial.println();
    Serial.println("   #  | kg  | dist_mm | min  | max  | jitter |  delta");
    printSep();

    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_POS; i++) {
        if (!table_[i].valid) continue;
        count++;

        bool    hasDelta = (i > 0 && table_[i - 1].valid);
        int16_t delta    = hasDelta
            ? (int16_t)table_[i].dist - (int16_t)table_[i - 1].dist
            : 0;

        char buf[72];
        if (hasDelta)
            snprintf(buf, sizeof(buf), "  %2d  | %3d | %7d | %4d | %4d | %6d | %+6d",
                i + 1, posKg(i),
                table_[i].dist, table_[i].dmin, table_[i].dmax, table_[i].jitter,
                delta);
        else
            snprintf(buf, sizeof(buf), "  %2d  | %3d | %7d | %4d | %4d | %6d |    ---",
                i + 1, posKg(i),
                table_[i].dist, table_[i].dmin, table_[i].dmax, table_[i].jitter);

        Serial.print(buf);
        if (table_[i].jitter > JITTER_WARN)                Serial.print("  !! JITTER");
        if (hasDelta && abs(delta) < (int16_t)DELTA_WARN)  Serial.print("  !! TOO CLOSE");
        Serial.println();
    }

    printSep();
    if (count == 0) { Serial.println("  No data yet."); Serial.println(); return; }

    bool clean = true;
    for (uint8_t i = 0; i < NUM_POS; i++) {
        if (!table_[i].valid) continue;
        if (table_[i].jitter > JITTER_WARN) {
            Serial.printf("  WARN pos %d (%d kg): jitter=%d mm\n",
                i + 1, posKg(i), table_[i].jitter);
            clean = false;
        }
        if (i > 0 && table_[i - 1].valid) {
            uint16_t gap = (uint16_t)abs((int16_t)table_[i].dist - (int16_t)table_[i - 1].dist);
            if (gap < DELTA_WARN) {
                Serial.printf("  WARN pos %d/%d: only %d mm apart\n", i, i + 1, gap);
                clean = false;
            }
        }
    }
    if (clean) Serial.println("  All positions look OK.");
    Serial.println();
}

void Calibrator::promptPos() {
    if (currentPos_ >= NUM_POS) {
        Serial.println();
        Serial.println("=== CALIBRATION COMPLETE ===");
        printTable();
        calibrating_ = false;
        return;
    }
    Serial.println();
    Serial.printf("--- [%d / %d]  Insert pin -> position %d  (%d kg) ---\n",
        currentPos_ + 1, NUM_POS, currentPos_ + 1, posKg(currentPos_));
    Serial.println("    Keep pin still, then press ENTER.");
}

// ─── Core measurement ─────────────────────────────────────

bool Calibrator::doMeasure(uint8_t idx) {
    if (!sensor_) return false;

    uint16_t buf[SAMPLES];
    uint8_t  valid = 0;

    Serial.print("  Sampling");
    for (uint8_t i = 0; i < SAMPLES; i++) {
        net::webServerHandle();   // keep web UI alive during measurement
        uint16_t d = sensor_->readDistance();
        if (d > 0 && d < 8190) buf[valid++] = d;
        Serial.print('.');
        delay(50);
    }
    Serial.println();

    if (valid < SAMPLES / 2) {
        Serial.println("  ERROR: too many bad readings.");
        return false;
    }

    // Insertion sort
    for (uint8_t i = 1; i < valid; i++) {
        uint16_t key = buf[i];
        int8_t   j   = (int8_t)i - 1;
        while (j >= 0 && buf[j] > key) { buf[j + 1] = buf[j]; j--; }
        buf[j + 1] = key;
    }

    uint16_t mn     = buf[0];
    uint16_t mx     = buf[valid - 1];
    uint16_t median = buf[valid / 2];
    uint16_t jitter = mx - mn;

    table_[idx] = { median, mn, mx, jitter, true };

    Serial.printf("  Median : %d mm\n", median);
    Serial.printf("  Min    : %d mm\n", mn);
    Serial.printf("  Max    : %d mm\n", mx);
    Serial.printf("  Jitter : %d mm%s\n",
        jitter, jitter > JITTER_WARN ? "  !! HIGH" : "");

    return true;
}

void Calibrator::resetAll() {
    for (uint8_t i = 0; i < NUM_POS; i++) table_[i].valid = false;
    currentPos_  = 0;
    calibrating_ = false;
    liveMode_    = false;
    Serial.println("Calibration data cleared.");
}

// ─── Command dispatch ─────────────────────────────────────

void Calibrator::handleCommand(char cmd) {
    switch (cmd) {

        case 'h':
            printHelp();
            break;

        case 'l':
            liveMode_ = !liveMode_;
            Serial.println(liveMode_ ? "Live mode ON (send 'l' to stop)." : "Live mode OFF.");
            break;

        case 'x':
            resetAll();
            break;

        case 'p':
            printTable();
            break;

        case 's':
            if (!calibrating_) {
                calibrating_ = true;
                currentPos_  = 0;
                Serial.println("Starting calibration...");
                promptPos();
            } else {
                Serial.println("Already in progress. ENTER=measure  r=repeat  x=reset");
            }
            break;

        case 'r':
            if (calibrating_) { promptPos(); }
            else { Serial.println("Not calibrating. Press 's' to start."); }
            break;

        case '\0':  // bare ENTER
            if (calibrating_) {
                if (doMeasure(currentPos_)) {
                    currentPos_++;
                    promptPos();
                } else {
                    Serial.println("Press ENTER to retry.");
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

// ─── Remote calibration (via backend) ────────────────────

void Calibrator::tickRemote() {
    uint32_t now = millis();

    // Push live distance every CALIB_PUSH_INTERVAL_MS
    if (now - remotePushTimer_ >= CALIB_PUSH_INTERVAL_MS) {
        remotePushTimer_ = now;
        if (sensor_) {
            uint16_t d = sensor_->readDistance();
            if (d > 0 && d < 8190) {
                net::calibPushDistance(d);
            }
        }
    }

    // Poll for commands every CALIB_POLL_INTERVAL_MS
    if (now - remotePollTimer_ >= CALIB_POLL_INTERVAL_MS) {
        remotePollTimer_ = now;
        net::CalibCommand cmd = net::calibPollCommand();
        if (cmd.hasCommand) {
            if (strcmp(cmd.command, "measure") == 0) {
                uint8_t idx = (uint8_t)cmd.position;
                if (idx < NUM_POS) {
                    Serial.printf("[CALIB] Remote measure command: pos=%d (%d kg)\n", idx, posKg(idx));
                    if (doMeasure(idx)) {
                        net::calibPostResult(
                            idx, posKg(idx),
                            table_[idx].dist, table_[idx].dmin,
                            table_[idx].dmax, table_[idx].jitter
                        );
                    }
                }
            }
        }
    }
}

void Calibrator::loadFromServer() {
    serverTableCount_ = net::calibLoadTable(serverTable_, NUM_POS);
}

int Calibrator::distToWeightKg(uint16_t distMm) const {
    if (serverTableCount_ == 0) return -1;

    int bestIdx = -1;
    int bestDiff = 999999;
    for (int i = 0; i < NUM_POS; i++) {
        if (!serverTable_[i].valid) continue;
        int diff = abs((int)distMm - serverTable_[i].distance_mm);
        if (diff < bestDiff) {
            bestDiff = diff;
            bestIdx = i;
        }
    }

    if (bestIdx < 0) return -1;
    // Only match if within reasonable range (half the jitter or 50mm, whichever is larger)
    if (bestDiff > 50) return -1;
    return serverTable_[bestIdx].weight_kg;
}

// ─── Main tick (called every loop) ────────────────────────

void Calibrator::tick() {
    // Remote calibration: push distance and poll commands
    tickRemote();

    // Live mode: print distance every 200 ms
    if (liveMode_) {
        if (millis() - liveTimer_ >= 200) {
            liveTimer_ = millis();
            if (sensor_) {
                uint16_t d = sensor_->readDistance();
                if (d > 0 && d < 8190) Serial.printf("LIVE: %4d mm\n", d);
                else                    Serial.println("LIVE:  ---");
            }
        }
    }

    // Serial input — non-blocking
    if (!Serial.available()) return;

    String line = Serial.readStringUntil('\n');
    line.trim();
    char cmd = line.length() > 0 ? (char)tolower((unsigned char)line[0]) : '\0';
    handleCommand(cmd);
}

}  // namespace calib
