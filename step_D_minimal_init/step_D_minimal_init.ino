// Step D — Minimal VL53L0X Init (no libraries)
// Performs the minimum register init for single-shot ranging.
// Each step prints OK/FAIL for easy debugging.

#include <Wire.h>

#define ADDR      0x29
#define XSHUT_PIN 16

static uint8_t stop_variable = 0;

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)1);
  uint8_t val = Wire.read();
  delay(2);
  return val;
}

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
  delay(2);
}

void writeReg16(uint8_t reg, uint16_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write((val >> 8) & 0xFF);
  Wire.write(val & 0xFF);
  Wire.endTransmission(true);
  delay(2);
}

bool doSingleRef(uint8_t vhv_init_byte) {
  writeReg(0x00, 0x01 | vhv_init_byte);
  delay(10);
  uint32_t t0 = millis();
  while ((readReg(0x13) & 0x07) == 0) {
    if (millis() - t0 > 1000) return false;
    delay(10);
  }
  writeReg(0x0B, 0x01);  // clear interrupt
  writeReg(0x00, 0x00);  // stop
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== Minimal Init ===");

  // Wake sensor
  pinMode(XSHUT_PIN, OUTPUT);
  digitalWrite(XSHUT_PIN, LOW);
  delay(200);
  digitalWrite(XSHUT_PIN, HIGH);
  delay(500);

  Wire.begin();
  Wire.setClock(100000);
  delay(100);

  // 1. Verify Model ID
  uint8_t model = readReg(0xC0);
  Serial.print("Model ID: 0x");
  Serial.print(model, HEX);
  if (model != 0xEE) {
    Serial.println("  <- FAIL (not VL53L0X)");
    return;
  }
  Serial.println("  <- OK");
  delay(10);

  // 2. DataInit — set 2V8 mode
  uint8_t vhv = readReg(0x89);
  writeReg(0x89, vhv | 0x01);
  Serial.println("2V8 mode: set");
  delay(10);

  // 3. I2C standard mode
  writeReg(0x88, 0x00);
  Serial.println("I2C standard mode: set");
  delay(10);

  // 4. Read stop_variable
  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  stop_variable = readReg(0x91);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);
  Serial.print("stop_variable: 0x");
  Serial.println(stop_variable, HEX);
  delay(10);

  // 5. Disable MSRC/TCC limit checks
  writeReg(0x60, readReg(0x60) | 0x12);
  Serial.println("MSRC/TCC disabled");
  delay(10);

  // 6. Signal rate limit 0.25 MCPS
  writeReg16(0x44, 32);
  Serial.println("Signal rate limit: set");
  delay(10);

  // 7. Sequence config
  writeReg(0x01, 0xE8);
  Serial.println("Sequence config: 0xE8");
  delay(10);

  // 8. GPIO config — new sample ready interrupt
  writeReg(0x0A, 0x04);
  uint8_t gpio_hv = readReg(0x84);
  writeReg(0x84, (gpio_hv & ~0x10) | 0x10);
  writeReg(0x0B, 0x01);
  Serial.println("GPIO interrupt: configured");
  delay(10);

  // 9. VHV calibration
  Serial.print("VHV cal... ");
  writeReg(0x01, 0x01);
  if (!doSingleRef(0x40)) {
    Serial.println("FAIL (timeout reading reg 0x13)");
    Serial.print("  reg 0x13 raw: 0x");
    Serial.println(readReg(0x13), HEX);
    return;
  }
  Serial.println("OK");
  delay(10);

  // 10. Phase calibration
  Serial.print("Phase cal... ");
  writeReg(0x01, 0x02);
  if (!doSingleRef(0x00)) {
    Serial.println("FAIL (timeout reading reg 0x13)");
    Serial.print("  reg 0x13 raw: 0x");
    Serial.println(readReg(0x13), HEX);
    return;
  }
  Serial.println("OK");
  delay(10);

  // 11. Restore full sequence config
  writeReg(0x01, 0xE8);
  Serial.println();
  Serial.println("Init complete!");
}

void loop() {}
