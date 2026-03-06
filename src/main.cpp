#include <Arduino.h>

// VL53L0X Direct Register Access — no libraries, just Wire.h
// Avoids ESP32 I2C driver crash from rapid bulk transactions

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

#define ADDR 0x29
#define XSHUT_PIN 16

// WiFi credentials — replace with your network
const char* WIFI_SSID = "profit_dev";
const char* WIFI_PASS = "UnzgmBlz7EwAU2aH5qVM";

WebServer server(80);
volatile uint16_t lastDistance = 0;

static uint8_t stop_variable = 0;

// --- I2C bus recovery ---
// Toggles SCL 16 times to release a stuck SDA line, then generates STOP.
void i2cBusRecovery() {
  pinMode(21, INPUT_PULLUP);  // SDA
  pinMode(22, OUTPUT);        // SCL
  for (int i = 0; i < 16; i++) {
    digitalWrite(22, LOW);
    delayMicroseconds(5);
    digitalWrite(22, HIGH);
    delayMicroseconds(5);
  }
  // Generate STOP: SDA low→high while SCL high
  pinMode(21, OUTPUT);
  digitalWrite(21, LOW);
  delayMicroseconds(5);
  digitalWrite(21, HIGH);
  delayMicroseconds(5);
  // Release both lines
  pinMode(21, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);
  delay(10);
}

// --- Low-level register I/O with delays ---

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)1);
  uint8_t val = Wire.read();
  delayMicroseconds(500);
  return val;
}

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
  delayMicroseconds(500);
}

uint16_t readReg16(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)2);
  uint16_t val = (uint16_t)Wire.read() << 8;
  val |= Wire.read();
  delayMicroseconds(500);
  return val;
}

void writeReg16(uint8_t reg, uint16_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write((val >> 8) & 0xFF);
  Wire.write(val & 0xFF);
  Wire.endTransmission(true);
  delayMicroseconds(500);
}

// --- Initialization ---

bool doSingleRef(uint8_t vhv_init_byte);

bool initSensor() {
  // 1. Verify Model ID
  uint8_t model = readReg(0xC0);
  Serial.print("Model ID: 0x");
  Serial.println(model, HEX);
  if (model != 0xEE) {
    Serial.println("ERROR: not VL53L0X");
    return false;
  }
  delay(10);

  // 2. DataInit — set 2V8 mode (sensor on VIN 5V, internal reg)
  uint8_t vhv = readReg(0x89);
  writeReg(0x89, vhv | 0x01);
  delay(10);

  // 3. Set I2C standard mode
  writeReg(0x88, 0x00);
  delay(10);

  // 4. Read stop_variable (needed for measurement start sequence)
  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  stop_variable = readReg(0x91);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);
  Serial.print("Stop variable: 0x");
  Serial.println(stop_variable, HEX);
  delay(10);

  // 5. Disable MSRC and TCC limit checks
  writeReg(0x60, readReg(0x60) | 0x12);
  delay(10);

  // 6. Set signal rate limit to 0.25 MCPS (fixed point 9.7: 0.25 * 128 = 32)
  writeReg16(0x44, 32);
  delay(10);

  // 7. Set sequence config — enable DSS, VHV, phasecal, final range
  writeReg(0x01, 0xE8);
  delay(10);

  // 8. GPIO config — new sample ready interrupt
  writeReg(0x0A, 0x04);
  uint8_t gpio_hv = readReg(0x84);
  writeReg(0x84, (gpio_hv & ~0x10) | 0x10);  // active low
  writeReg(0x0B, 0x01);  // clear interrupt
  delay(10);

  // 9. Do one VHV calibration (measurement timeout type 0x01)
  Serial.print("VHV cal... ");
  writeReg(0x01, 0x01);  // only VHV
  if (!doSingleRef(0x40)) { Serial.println("FAIL"); return false; }
  Serial.println("OK");
  delay(10);

  // 10. Phase calibration (measurement timeout type 0x02)
  Serial.print("Phase cal... ");
  writeReg(0x01, 0x02);  // only phase cal
  if (!doSingleRef(0x00)) { Serial.println("FAIL"); return false; }
  Serial.println("OK");
  delay(10);

  // 11. Restore full sequence config
  writeReg(0x01, 0xE8);
  delay(10);

  Serial.println("Init OK");
  return true;
}

// Perform a single ref calibration measurement
bool doSingleRef(uint8_t vhv_init_byte) {
  writeReg(0x00, 0x01 | vhv_init_byte);  // SYSRANGE_START with cal flag
  delay(10);

  // Wait for measurement complete
  uint32_t t0 = millis();
  while ((readReg(0x13) & 0x07) == 0) {
    if (millis() - t0 > 1000) return false;
    delay(10);
  }
  writeReg(0x0B, 0x01);  // clear interrupt
  writeReg(0x00, 0x00);  // stop
  return true;
}

// --- Single-shot measurement ---

uint16_t readDistance() {
  // Pololu sequence: preamble to set stop_variable
  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  writeReg(0x91, stop_variable);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);
  delay(5);

  // Start single-shot range measurement
  writeReg(0x00, 0x01);

  // Wait for SYSRANGE_START bit 0 to clear (measurement started)
  uint32_t t0 = millis();
  while (readReg(0x00) & 0x01) {
    if (millis() - t0 > 500) {
      Serial.println("Start timeout!");
      return 0xFFFF;
    }
    delay(5);
  }

  // Wait for result ready (interrupt status)
  t0 = millis();
  while ((readReg(0x13) & 0x07) == 0) {
    if (millis() - t0 > 500) {
      Serial.println("Meas timeout!");
      return 0xFFFF;
    }
    delay(5);
  }

  // Read range result (16-bit at register 0x14+0x0A = 0x1E)
  uint16_t dist = readReg16(0x14 + 0x0A);

  // Clear interrupt
  writeReg(0x0B, 0x01);

  return dist;
}

// --- Median filter (3 samples, kills spikes) ---

#define FILTER_SIZE 3
#define DIST_MIN 30
#define DIST_MAX 2000

uint16_t filterBuf[FILTER_SIZE];
uint8_t filterIdx = 0;
bool filterFull = false;

uint16_t medianOfThree(uint16_t a[]) {
  uint16_t x = a[0], y = a[1], z = a[2];
  if (x > y) { uint16_t t = x; x = y; y = t; }
  if (y > z) { uint16_t t = y; y = z; z = t; }
  if (x > y) { uint16_t t = x; x = y; y = t; }
  return y;
}

uint16_t filteredDistance() {
  uint16_t raw = readDistance();
  if (raw == 0xFFFF) return 0xFFFF;

  // Clamp out-of-range to last good value
  if (raw < DIST_MIN || raw > DIST_MAX) {
    if (filterFull) return medianOfThree(filterBuf);
    return 0xFFFF;
  }

  filterBuf[filterIdx] = raw;
  filterIdx = (filterIdx + 1) % FILTER_SIZE;
  if (filterIdx == 0) filterFull = true;

  if (!filterFull) return raw;
  return medianOfThree(filterBuf);
}

// --- Full sensor reset (XSHUT cycle + re-init) ---

bool resetSensor() {
  Serial.println("Resetting sensor...");
  digitalWrite(XSHUT_PIN, LOW);
  delay(200);
  digitalWrite(XSHUT_PIN, HIGH);
  delay(500);
  i2cBusRecovery();
  Wire.begin();
  Wire.setClock(100000);
  delay(100);

  Wire.beginTransmission(ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("Sensor not responding after reset");
    return false;
  }
  return initSensor();
}

// --- Web server handlers ---

void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to open index.html");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleDistance() {
  String json = "{\"distance_mm\":" + String(lastDistance) + ",\"t\":" + String(millis()) + "}";
  server.send(200, "application/json", json);
}

// --- Main ---

bool sensorOK = false;
int errorCount = 0;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== VL53L0X Direct ===");

  // Hard-reset sensor via XSHUT
  pinMode(XSHUT_PIN, OUTPUT);
  digitalWrite(XSHUT_PIN, LOW);
  delay(200);
  digitalWrite(XSHUT_PIN, HIGH);
  delay(500);  // sensor needs time to boot after XSHUT release

  // Recover I2C bus in case SDA is stuck from a previous reset
  i2cBusRecovery();

  Wire.begin();
  Wire.setClock(100000);
  delay(100);

  // I2C bus scan
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

  // Retry sensor detection with XSHUT power-cycle between attempts
  bool found = false;
  for (int attempt = 0; attempt < 5; attempt++) {
    Wire.beginTransmission(ADDR);
    if (Wire.endTransmission() == 0) {
      found = true;
      break;
    }
    Serial.print("Retry ");
    Serial.print(attempt + 1);
    Serial.println(" — cycling XSHUT...");

    // Power-cycle sensor via XSHUT
    digitalWrite(XSHUT_PIN, LOW);
    delay(200);
    digitalWrite(XSHUT_PIN, HIGH);
    delay(500);
    i2cBusRecovery();
    Wire.begin();
    Wire.setClock(100000);
    delay(100);
  }

  if (!found) {
    Serial.println("Sensor not found at 0x29!");
    return;
  }
  Serial.println("Sensor found at 0x29");
  delay(100);

  sensorOK = initSensor();

  // Mount LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
  }

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Start web server
  server.on("/", handleRoot);
  server.on("/distance", handleDistance);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  if (!sensorOK) {
    sensorOK = resetSensor();
    if (!sensorOK) {
      delay(3000);
      return;
    }
    errorCount = 0;
  }

  uint16_t dist = filteredDistance();
  if (dist == 0xFFFF) {
    errorCount++;
    Serial.print("Error count: ");
    Serial.println(errorCount);
    if (errorCount >= 3) {
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

  delay(50);
}
