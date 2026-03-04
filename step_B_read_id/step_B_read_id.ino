// Step B — Read VL53L0X ID Registers
// Verifies I2C read works. Expect Model ID 0xEE at reg 0xC0.

#include <Wire.h>

#define ADDR      0x29
#define XSHUT_PIN 16

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  uint8_t err = Wire.endTransmission(false);  // repeated start
  if (err != 0) {
    Serial.print("  I2C error on reg 0x");
    Serial.print(reg, HEX);
    Serial.print(": ");
    switch (err) {
      case 2: Serial.println("NACK on address"); break;
      case 3: Serial.println("NACK on data"); break;
      case 4: Serial.println("other error"); break;
      default: Serial.println(err); break;
    }
    return 0xFF;
  }
  Wire.requestFrom(ADDR, (uint8_t)1);
  return Wire.read();
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== VL53L0X ID Check ===");

  // Wake sensor
  pinMode(XSHUT_PIN, OUTPUT);
  digitalWrite(XSHUT_PIN, LOW);
  delay(200);
  digitalWrite(XSHUT_PIN, HIGH);
  delay(500);

  Wire.begin();
  Wire.setClock(100000);
  delay(100);

  // Check sensor present
  Wire.beginTransmission(ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("Sensor not found at 0x29! Run Step A first.");
    return;
  }

  // Read ID registers
  uint8_t modelId   = readReg(0xC0);
  uint8_t moduleType = readReg(0xC1);
  uint8_t revision   = readReg(0xC2);

  Serial.print("Reg 0xC0 (Model ID):    0x");
  Serial.print(modelId, HEX);
  Serial.println(modelId == 0xEE ? "  <- OK" : "  <- UNEXPECTED!");

  Serial.print("Reg 0xC1 (Module Type): 0x");
  Serial.println(moduleType, HEX);

  Serial.print("Reg 0xC2 (Revision):    0x");
  Serial.println(revision, HEX);

  if (modelId == 0xFF && moduleType == 0xFF && revision == 0xFF) {
    Serial.println();
    Serial.println("All 0xFF — SDA/SCL problem (pullups, level, contact)");
  } else if (modelId == 0x00 && moduleType == 0x00 && revision == 0x00) {
    Serial.println();
    Serial.println("All 0x00 — SDA line stuck low?");
  }
}

void loop() {}
