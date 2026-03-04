// Step A — I2C Bus Scan
// Finds all I2C devices. Expect 0x29 (VL53L0X).

#include <Wire.h>

#define XSHUT_PIN 16

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== I2C Scan ===");

  // Wake sensor: XSHUT LOW→HIGH
  pinMode(XSHUT_PIN, OUTPUT);
  digitalWrite(XSHUT_PIN, LOW);
  delay(200);
  digitalWrite(XSHUT_PIN, HIGH);
  delay(500);

  // Wire.begin() without args — known ESP32 fix for boot loop
  Wire.begin();
  Wire.setClock(100000);
  delay(100);

  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("Found: 0x");
      Serial.println(addr, HEX);
      found++;
    }
  }

  Serial.print("Scan done. Devices: ");
  Serial.println(found);

  if (found == 0) {
    Serial.println();
    Serial.println("--- No devices found! Checklist: ---");
    Serial.println("1. GND sensor -> GND ESP32?");
    Serial.println("2. SDA/SCL not swapped? (try swapping wires)");
    Serial.println("3. Power: VIN (5V), NOT 3.3V");
    Serial.println("4. XSHUT pulled HIGH? (GPIO16 -> HIGH in code)");
    Serial.println("5. If boot loop: Wire.begin() without args");
  }
}

void loop() {
  // nothing — one-shot test
}
