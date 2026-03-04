// Step E — Single-Shot Measurement (no libraries)
// Full init + repeated single-shot distance readings every 500ms.

#include <Wire.h>

#define ADDR      0x29
#define XSHUT_PIN 16

static uint8_t stop_variable = 0;

// --- Low-level I/O ---

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

uint16_t readReg16(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)2);
  uint16_t val = (uint16_t)Wire.read() << 8;
  val |= Wire.read();
  delay(2);
  return val;
}

void writeReg16(uint8_t reg, uint16_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write((val >> 8) & 0xFF);
  Wire.write(val & 0xFF);
  Wire.endTransmission(true);
  delay(2);
}

// --- Calibration helper ---

bool doSingleRef(uint8_t vhv_init_byte) {
  writeReg(0x00, 0x01 | vhv_init_byte);
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

// --- Init (same as Step D) ---

bool initSensor() {
  uint8_t model = readReg(0xC0);
  Serial.print("Model ID: 0x");
  Serial.print(model, HEX);
  if (model != 0xEE) { Serial.println(" <- FAIL"); return false; }
  Serial.println(" <- OK");
  delay(10);

  writeReg(0x89, readReg(0x89) | 0x01);  // 2V8 mode
  writeReg(0x88, 0x00);                   // I2C standard mode
  delay(10);

  // Read stop_variable
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

  writeReg(0x60, readReg(0x60) | 0x12);  // disable MSRC/TCC
  writeReg16(0x44, 32);                   // signal rate limit
  writeReg(0x01, 0xE8);                   // sequence config
  delay(10);

  // GPIO interrupt config
  writeReg(0x0A, 0x04);
  uint8_t gpio_hv = readReg(0x84);
  writeReg(0x84, (gpio_hv & ~0x10) | 0x10);
  writeReg(0x0B, 0x01);
  delay(10);

  // VHV calibration
  Serial.print("VHV cal... ");
  writeReg(0x01, 0x01);
  if (!doSingleRef(0x40)) { Serial.println("FAIL"); return false; }
  Serial.println("OK");
  delay(10);

  // Phase calibration
  Serial.print("Phase cal... ");
  writeReg(0x01, 0x02);
  if (!doSingleRef(0x00)) { Serial.println("FAIL"); return false; }
  Serial.println("OK");
  delay(10);

  writeReg(0x01, 0xE8);  // restore sequence config
  Serial.println("Init complete!");
  return true;
}

// --- Single-shot measurement ---

uint16_t readDistance() {
  // Preamble (stop_variable sequence)
  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  writeReg(0x91, stop_variable);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);
  delay(5);

  // Start single-shot
  writeReg(0x00, 0x01);

  // Wait for start clear (bit 0)
  uint32_t t0 = millis();
  while (readReg(0x00) & 0x01) {
    if (millis() - t0 > 500) {
      Serial.println("Start timeout! (reg 0x00 not clearing)");
      Serial.print("  reg 0x00 = 0x");
      Serial.println(readReg(0x00), HEX);
      return 0xFFFF;
    }
    delay(5);
  }

  // Wait for result ready
  t0 = millis();
  while ((readReg(0x13) & 0x07) == 0) {
    if (millis() - t0 > 500) {
      Serial.println("Measurement timeout! (reg 0x13 not ready)");
      Serial.print("  reg 0x13 = 0x");
      Serial.println(readReg(0x13), HEX);
      Serial.print("  reg 0x01 = 0x");
      Serial.println(readReg(0x01), HEX);
      return 0xFFFF;
    }
    delay(5);
  }

  // Read 16-bit distance from reg 0x1E (0x14 + 0x0A)
  uint16_t dist = readReg16(0x14 + 0x0A);

  // Read range status for diagnostics
  uint8_t status = readReg(0x14) >> 3;

  // Clear interrupt
  writeReg(0x0B, 0x01);

  // Print result
  Serial.print("Distance: ");
  Serial.print(dist);
  Serial.print(" mm");
  if (status == 0) {
    Serial.println("  (status OK)");
  } else if (dist == 0 || dist >= 8190) {
    Serial.println("  (out of range)");
  } else {
    Serial.print("  (status: ");
    Serial.print(status);
    Serial.println(")");
  }

  return dist;
}

// --- Main ---

bool sensorOK = false;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== Single Shot ===");

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
    Serial.println("Sensor not found at 0x29!");
    return;
  }

  sensorOK = initSensor();
}

void loop() {
  if (!sensorOK) return;
  readDistance();
  delay(500);
}
