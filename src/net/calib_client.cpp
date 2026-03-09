#include "calib_client.h"
#include "../config.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <ArduinoJson.h>

namespace net {

static WiFiClientSecure secClient;
static bool secInit = false;

static void ensureClient() {
    if (!secInit) {
        secClient.setInsecure();
        secInit = true;
    }
}

void calibPushDistance(uint16_t distMm) {
    if (WiFi.status() != WL_CONNECTED) return;
    ensureClient();

    HTTPClient http;
    String url = String(BACKEND_BASE) + "/api/calib/live";
    http.begin(secClient, url);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"machine_id\":\"";
    body += MACHINE_ID;
    body += "\",\"distance_mm\":";
    body += distMm;
    body += "}";

    int code = http.POST(body);
    if (code <= 0) {
        Serial.print("[CALIB] push dist failed: ");
        Serial.println(http.errorToString(code));
    }
    http.end();
}

CalibCommand calibPollCommand() {
    CalibCommand result = { false, "", 0 };
    if (WiFi.status() != WL_CONNECTED) return result;
    ensureClient();

    HTTPClient http;
    String url = String(BACKEND_BASE) + "/api/calib/command?machine_id=" + MACHINE_ID;
    http.begin(secClient, url);

    int code = http.GET();
    if (code == 200) {
        String payload = http.getString();
        JsonDocument doc;
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            if (!doc["command"].isNull()) {
                result.hasCommand = true;
                strlcpy(result.command, doc["command"].as<const char*>(), sizeof(result.command));
                result.position = doc["position"] | 0;
            }
        }
    }
    http.end();
    return result;
}

void calibPostResult(int position, int weightKg, int distMm, int dmin, int dmax, int jitter) {
    if (WiFi.status() != WL_CONNECTED) return;
    ensureClient();

    HTTPClient http;
    String url = String(BACKEND_BASE) + "/api/calib/result";
    http.begin(secClient, url);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"machine_id\":\"";
    body += MACHINE_ID;
    body += "\",\"position\":";
    body += position;
    body += ",\"weight_kg\":";
    body += weightKg;
    body += ",\"distance_mm\":";
    body += distMm;
    body += ",\"dist_min\":";
    body += dmin;
    body += ",\"dist_max\":";
    body += dmax;
    body += ",\"jitter\":";
    body += jitter;
    body += "}";

    int code = http.POST(body);
    if (code > 0) {
        Serial.printf("[CALIB] result posted pos=%d → %d\n", position, code);
    } else {
        Serial.print("[CALIB] result post failed: ");
        Serial.println(http.errorToString(code));
    }
    http.end();
}

int calibLoadTable(CalibPoint* table, int maxPoints) {
    for (int i = 0; i < maxPoints; i++) table[i].valid = false;
    if (WiFi.status() != WL_CONNECTED) return 0;
    ensureClient();

    HTTPClient http;
    String url = String(BACKEND_BASE) + "/api/calib/table?machine_id=" + MACHINE_ID;
    http.begin(secClient, url);

    int code = http.GET();
    int loaded = 0;
    if (code == 200) {
        String payload = http.getString();
        JsonDocument doc;
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            JsonArray arr = doc.as<JsonArray>();
            for (JsonObject obj : arr) {
                int pos = obj["position"] | -1;
                if (pos < 0 || pos >= maxPoints) continue;
                table[pos].position = pos;
                table[pos].weight_kg = obj["weight_kg"] | 0;
                table[pos].distance_mm = obj["distance_mm"] | 0;
                table[pos].valid = true;
                loaded++;
            }
        }
        Serial.printf("[CALIB] loaded %d calibration points from server\n", loaded);
    } else {
        Serial.print("[CALIB] load table failed: ");
        Serial.println(code);
    }
    http.end();
    return loaded;
}

} // namespace net
