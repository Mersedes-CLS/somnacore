#include <Arduino.h>
#include "config.h"
#include "hal/i2c_bus.h"
#include "drivers/vl53l0x.h"
#include "processing/filter.h"
#include "net/wifi_manager.h"
#include "net/web_server.h"

static VL53L0X sensor(ADDR_VL53L0X, PIN_XSHUT_TOP);
static MedianFilter filter;
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
    net::webServerInit(&lastDistance);
}

void loop() {
    net::webServerHandle();

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
    if (dist != 0) {
        Serial.print("Distance: ");
        Serial.print(dist);
        Serial.println(" mm");
    }

    delay(LOOP_DELAY_MS);
}
