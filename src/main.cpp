#include <Arduino.h>

// VL53L0X Direct Register Access — no libraries, just Wire.h
// Avoids ESP32 I2C driver crash from rapid bulk transactions

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

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

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>SmartStack</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800;900&display=swap" rel="stylesheet">
<style>
:root {
  --bg: #0B0F14;
  --card: #141922;
  --card-hover: #1B2130;
  --purple: #8B5CF6;
  --purple-glow: #A78BFA;
  --purple-deep: #6D28D9;
  --cyan: #22D3EE;
  --text: #E5E7EB;
  --text2: #9CA3AF;
  --success: #22C55E;
  --warn: #F59E0B;
  --err: #EF4444;
  --radius: 16px;
}
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
html { height: 100%; }
body {
  background: var(--bg);
  color: var(--text);
  font-family: 'Inter', -apple-system, system-ui, sans-serif;
  min-height: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 24px 20px env(safe-area-inset-bottom, 20px);
  -webkit-font-smoothing: antialiased;
}

/* --- Layout shell --- */
.shell {
  width: 100%;
  max-width: 480px;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

/* --- Header --- */
.header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 4px;
}
.brand {
  font-size: 0.85em;
  font-weight: 600;
  color: var(--text2);
  letter-spacing: 0.08em;
  text-transform: uppercase;
}
.controls {
  display: flex;
  gap: 6px;
}
.btn {
  background: var(--card);
  color: var(--text2);
  border: 1px solid transparent;
  border-radius: 8px;
  padding: 6px 14px;
  font-family: inherit;
  font-size: 0.78em;
  font-weight: 500;
  cursor: pointer;
  transition: background 0.15s, border-color 0.15s;
}
.btn:hover { background: var(--card-hover); }
.btn.active {
  background: var(--purple-deep);
  color: #fff;
  border-color: var(--purple);
}

/* --- Hero: REPS --- */
.hero {
  background: var(--card);
  border-radius: var(--radius);
  padding: 40px 24px 36px;
  text-align: center;
  position: relative;
  overflow: hidden;
}
.hero::before {
  content: '';
  position: absolute;
  top: -1px; left: -1px; right: -1px; bottom: -1px;
  border-radius: var(--radius);
  border: 1px solid rgba(139,92,246,0.15);
  pointer-events: none;
}
.hero-label {
  font-size: 0.7em;
  font-weight: 600;
  color: var(--text2);
  letter-spacing: 0.12em;
  text-transform: uppercase;
  margin-bottom: 8px;
}
.hero-value {
  font-size: 7em;
  font-weight: 900;
  line-height: 1;
  color: #fff;
  text-shadow: 0 0 40px rgba(139,92,246,0.3), 0 0 80px rgba(139,92,246,0.1);
}
.hero-sub {
  font-size: 0.8em;
  color: var(--text2);
  margin-top: 12px;
  font-weight: 500;
}
.hero-sub span { color: var(--purple-glow); font-weight: 600; }

/* --- ROM bar --- */
.rom-card {
  background: var(--card);
  border-radius: var(--radius);
  padding: 20px 24px;
}
.rom-head {
  display: flex;
  justify-content: space-between;
  align-items: baseline;
  margin-bottom: 14px;
}
.rom-label {
  font-size: 0.7em;
  font-weight: 600;
  color: var(--text2);
  letter-spacing: 0.1em;
  text-transform: uppercase;
}
.rom-value {
  font-size: 1.1em;
  font-weight: 700;
  color: var(--text);
  font-variant-numeric: tabular-nums;
}
.rom-value .unit { font-weight: 400; color: var(--text2); font-size: 0.8em; margin-left: 2px; }
.rom-track {
  width: 100%;
  height: 6px;
  background: var(--card-hover);
  border-radius: 3px;
  overflow: hidden;
  position: relative;
}
.rom-fill {
  height: 100%;
  border-radius: 3px;
  background: linear-gradient(90deg, var(--purple-deep), var(--purple));
  box-shadow: 0 0 12px rgba(139,92,246,0.4);
  transition: width 0.15s ease-out;
  width: 0%;
}

/* --- Bottom row: weight + state --- */
.info-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 12px;
}
.info-card {
  background: var(--card);
  border-radius: var(--radius);
  padding: 18px 20px;
}
.info-label {
  font-size: 0.65em;
  font-weight: 600;
  color: var(--text2);
  letter-spacing: 0.1em;
  text-transform: uppercase;
  margin-bottom: 6px;
}
.info-value {
  font-size: 1.6em;
  font-weight: 700;
  font-variant-numeric: tabular-nums;
}
.info-value .unit { font-weight: 400; color: var(--text2); font-size: 0.55em; margin-left: 3px; }
#v_weight { color: var(--text); }

/* State chip */
#v_state {
  display: inline-block;
  font-size: 0.78em;
  font-weight: 600;
  letter-spacing: 0.06em;
  padding: 5px 14px;
  border-radius: 20px;
  text-transform: uppercase;
}
.state-ready { background: rgba(139,92,246,0.12); color: var(--purple-glow); }
.state-set { background: rgba(34,197,94,0.12); color: var(--success); }
.state-rest { background: rgba(245,158,11,0.12); color: var(--warn); }
.state-off { background: rgba(239,68,68,0.12); color: var(--err); }

/* --- Responsive: desktop wider --- */
@media (min-width: 640px) {
  .shell { max-width: 520px; gap: 20px; }
  body { padding: 40px 24px; }
  .hero { padding: 52px 32px 44px; }
  .hero-value { font-size: 9em; }
}

/* --- Small phones --- */
@media (max-width: 380px) {
  .hero-value { font-size: 5.5em; }
  .hero { padding: 32px 16px 28px; }
  .info-value { font-size: 1.3em; }
}
</style>
</head>
<body>
<div class="shell">
  <!-- Header -->
  <div class="header">
    <div class="brand">SmartStack</div>
    <div class="controls">
      <button class="btn" onclick="resetAll()">Reset</button>
      <button class="btn" id="btnPause" onclick="togglePause()">Pause</button>
    </div>
  </div>

  <!-- REPS hero -->
  <div class="hero">
    <div class="hero-label">Reps</div>
    <div class="hero-value" id="v_reps">0</div>
    <div class="hero-sub">Set <span id="v_setnum">1</span></div>
  </div>

  <!-- ROM indicator -->
  <div class="rom-card">
    <div class="rom-head">
      <div class="rom-label">Range of Motion</div>
      <div class="rom-value" id="v_rom">--<span class="unit">mm</span></div>
    </div>
    <div class="rom-track"><div class="rom-fill" id="romFill"></div></div>
  </div>

  <!-- Weight + State -->
  <div class="info-row">
    <div class="info-card">
      <div class="info-label">Weight</div>
      <div class="info-value" id="v_weight">--<span class="unit">kg</span></div>
    </div>
    <div class="info-card">
      <div class="info-label">Status</div>
      <div><span id="v_state" class="state-ready">READY</span></div>
    </div>
  </div>
</div>

<script>
const MAX_PTS = 200;
const data = [];
let reps = 0, paused = false;
// Rep detection — percentage thresholds
let baseline = null, inRep = false, repStartTime = 0, lastT = 0;
const REP_ENTER_PCT = 0.30;
const REP_EXIT_PCT = 0.10;
const REP_MIN_MS = 300;
const STABLE_COUNT = 3;
// Set tracking
let sets = 0, setReps = 0, lastRepTime = 0;
let setHistory = [];
const SET_TIMEOUT = 30000;
// ROM tracking
let romMin = Infinity, romMax = -Infinity;

function setState(s) {
  const el = document.getElementById('v_state');
  el.textContent = s;
  el.className = s === 'IN SET' ? 'state-set' : s === 'REST' ? 'state-rest' : s === 'OFF' ? 'state-off' : 'state-ready';
}

function updateROM(dist) {
  if (!inRep) { romMin = Infinity; romMax = -Infinity; return; }
  if (dist < romMin) romMin = dist;
  if (dist > romMax) romMax = dist;
  const range = romMax - romMin;
  document.getElementById('v_rom').innerHTML = range + '<span class="unit">mm</span>';
  // Fill bar: percentage of baseline as ROM
  const pct = baseline ? Math.min(range / baseline * 100 * 2, 100) : 0;
  document.getElementById('romFill').style.width = pct + '%';
}

function detectRep(dist) {
  if (baseline === null) {
    if (data.length >= STABLE_COUNT) {
      baseline = data.slice(-STABLE_COUNT).reduce((a, b) => a + b, 0) / STABLE_COUNT;
    }
    return;
  }
  const delta = baseline - dist;
  const enterThresh = baseline * REP_ENTER_PCT;
  const exitThresh = baseline * REP_EXIT_PCT;
  if (!inRep && delta > enterThresh) {
    inRep = true;
    repStartTime = Date.now();
    setState('IN SET');
  } else if (inRep && delta < exitThresh) {
    if (Date.now() - repStartTime >= REP_MIN_MS) {
      reps++;
      setReps++;
      lastRepTime = Date.now();
      document.getElementById('v_reps').textContent = setReps;
    }
    inRep = false;
  }
  updateROM(dist);
  if (!inRep) {
    baseline = baseline * 0.98 + dist * 0.02;
  }
}

async function poll() {
  if (paused) return;
  try {
    const r = await fetch('/distance');
    const d = await r.json();
    const dist = d.distance_mm;
    const stale = (lastT > 0 && d.t === lastT);
    lastT = d.t;
    if (stale) return;
    data.push(dist);
    if (data.length > MAX_PTS) data.shift();
    detectRep(dist);
    // Auto-close set after timeout
    if (setReps > 0 && lastRepTime > 0 && Date.now() - lastRepTime > SET_TIMEOUT) {
      sets++;
      setHistory.push({reps: setReps});
      setReps = 0;
      lastRepTime = 0;
      document.getElementById('v_reps').textContent = '0';
      document.getElementById('v_setnum').textContent = sets + 1;
      setState('REST');
    }
    // State transitions
    if (setReps === 0 && !inRep && sets === 0) setState('READY');
    else if (setReps > 0 && !inRep) setState('IN SET');
  } catch(e) {
    setState('OFF');
  }
}

function resetAll() {
  reps = 0; baseline = null; inRep = false;
  sets = 0; setReps = 0; lastRepTime = 0; setHistory = [];
  romMin = Infinity; romMax = -Infinity;
  document.getElementById('v_reps').textContent = '0';
  document.getElementById('v_setnum').textContent = '1';
  document.getElementById('v_rom').innerHTML = '--<span class="unit">mm</span>';
  document.getElementById('romFill').style.width = '0%';
  document.getElementById('v_weight').innerHTML = '--<span class="unit">kg</span>';
  setState('READY');
}
function togglePause() {
  paused = !paused;
  document.getElementById('btnPause').textContent = paused ? 'Resume' : 'Pause';
  document.getElementById('btnPause').classList.toggle('active', paused);
}

setInterval(poll, 150);
poll();
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
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
