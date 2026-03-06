#include "i2c_bus.h"
#include "../config.h"
#include <Arduino.h>
#include <Wire.h>

namespace hal {

void i2cBusRecovery() {
    pinMode(PIN_SDA, INPUT_PULLUP);
    pinMode(PIN_SCL, OUTPUT);
    for (int i = 0; i < 16; i++) {
        digitalWrite(PIN_SCL, LOW);
        delayMicroseconds(5);
        digitalWrite(PIN_SCL, HIGH);
        delayMicroseconds(5);
    }
    // Generate STOP: SDA low→high while SCL high
    pinMode(PIN_SDA, OUTPUT);
    digitalWrite(PIN_SDA, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_SDA, HIGH);
    delayMicroseconds(5);
    // Release both lines
    pinMode(PIN_SDA, INPUT_PULLUP);
    pinMode(PIN_SCL, INPUT_PULLUP);
    delay(10);
}

void i2cInit() {
    Wire.begin();
    Wire.setClock(I2C_CLOCK_HZ);
    delay(100);
}

int i2cScan() {
    Serial.println("I2C scan:");
    int devCount = 0;
    for (byte a = 1; a < 127; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {
            Serial.print("  Found: 0x");
            Serial.println(a, HEX);
            devCount++;
        }
    }
    if (devCount == 0) Serial.println("  (none)");
    delay(100);
    return devCount;
}

}  // namespace hal
