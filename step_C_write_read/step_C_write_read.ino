// Step C — Write/Read Register Test
// Writes to reg 0x80 and reads back. Verifies I2C write path.

#include <Wire.h>

#define ADDR      0x29
#define XSHUT_PIN 16

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)1);
  return Wire.read();
}

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

bool testWriteRead(uint8_t reg, uint8_t val) {
  writeReg(reg, val);
  delay(5);
  uint8_t back = readReg(reg);
  Serial.print("Write 0x");
  Serial.print(reg, HEX);
  Serial.print(" = 0x");
  Serial.print(val, HEX);
  Serial.print(", Read back: 0x");
  Serial.print(back, HEX);
  if (back == val) {
    Serial.println("  <- OK");
    return true;
  } else {
    Serial.println("  <- MISMATCH!");
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== Write/Read Test ===");

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

  // Test reg 0x80 (safe scratch register used by Pololu init)
  bool ok1 = testWriteRead(0x80, 0x01);
  bool ok2 = testWriteRead(0x80, 0x00);

  Serial.println();
  if (ok1 && ok2) {
    Serial.println("I2C write/read: OK");
  } else {
    Serial.println("I2C write/read: FAILED — check SDA line");
  }
}

void loop() {}
