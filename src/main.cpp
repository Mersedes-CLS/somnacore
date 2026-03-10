#include <Arduino.h>
#include "config.h"
#include "hal/i2c_bus.h"
#include "drivers/vl53l0x.h"
#include "processing/filter.h"
#include "processing/rep_detector.h"
#include "app/session.h"
#include "net/wifi_manager.h"
#include "net/web_server.h"
#include "calib/calibrator.h"

static VL53L0X sensor(ADDR_VL53L0X, PIN_XSHUT_TOP);
static MedianFilter filter;
static RepDetector repDetector;
static Session session;
static calib::Calibrator calibrator;
static volatile uint16_t lastDistance = 0;
static bool sensorOK = false;
static int errorCount = 0;

void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("=== VL53L0X Direct ===");

    // Hard-reset sensor via XSHUT
    sensor.xshutReset();

    // Recover I2C bus in case SDA is stuck from a previous reset
    hal::i2cBusRecovery();
    hal::i2cInit();
    hal::i2cScan();

    // Retry sensor detection with XSHUT power-cycle between attempts
    bool found = false;
    for (int attempt = 0; attempt < MAX_INIT_RETRIES; attempt++) {
        if (sensor.probeAddress()) { found = true; break; }
        Serial.print("Retry ");
        Serial.print(attempt + 1);
        Serial.println(" — cycling XSHUT...");
        sensor.xshutReset();
        hal::i2cBusRecovery();
        hal::i2cInit();
    }

    if (!found) {
        Serial.println("Sensor not found at 0x29!");
    } else {
        Serial.println("Sensor found at 0x29");
        delay(100);
        sensorOK = sensor.init();
    }

    net::wifiConnect();
    net::webServerInit(&lastDistance, &session);
    calibrator.begin(&sensor);
    calibrator.loadFromServer();
    session.setCalibrator(&calibrator);
}

void loop() {
    net::webServerHandle();
    calibrator.tick();

    if (!sensorOK) {
        sensorOK = sensor.reset();
        if (!sensorOK) { delay(3000); return; }
        errorCount = 0;
    }

    uint16_t dist = filter.update(sensor.readDistance());
    if (dist == 0xFFFF) {
        errorCount++;
        Serial.print("Error count: ");
        Serial.println(errorCount);
        if (errorCount >= MAX_SENSOR_ERRORS) {
            Serial.println("Too many errors, resetting sensor...");
            sensorOK = false;
        }
        delay(100);
        return;
    }

    errorCount = 0;
    lastDistance = dist;

    // Accumulate distance for weight determination during active set
    if (session.state() == SessionState::IN_SET) {
        session.feedDistance(dist);
    }

    if (dist != 0) {
        if (repDetector.update(dist)) {
            session.feedRep(repDetector.peakDelta());
            Serial.print("[REP] #");
            Serial.print(session.currentReps());
            Serial.print(" ROM=");
            Serial.print(repDetector.peakDelta());
            Serial.println("mm");
        }

        uint8_t prevSets = session.setCount();
        session.tick();
        if (session.setCount() > prevSets) {
            const SetRecord* h = session.history();
            const SetRecord& last = h[prevSets];
            Serial.print("[SET] #");
            Serial.print(session.setCount());
            Serial.print(" complete: ");
            Serial.print(last.reps);
            Serial.print(" reps, ROM=");
            Serial.print(last.romMm);
            Serial.println("mm");
        }
    }

    delay(LOOP_DELAY_MS);
}
