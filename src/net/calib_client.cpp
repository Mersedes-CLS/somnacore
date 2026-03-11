#include "calib_client.h"
#include "../config.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <ArduinoJson.h>

namespace net {

void calibPostResult(int position, int weightKg, int distMm, int dmin, int dmax, int jitter) {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    String url = String(BACKEND_BASE) + "/api/calib/result";
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);

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
    Serial.printf("[CALIB] result posted pos=%d -> %d\n", position, code);
    http.end();
}

void calibPushLive(uint16_t distMm, int weightKg) {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    String url = String(BACKEND_BASE) + "/api/calib/live";
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(3000);

    String body = "{\"machine_id\":\"";
    body += MACHINE_ID;
    body += "\",\"distance_mm\":";
    body += distMm;
    body += ",\"weight_kg\":";
    body += weightKg;
    body += "}";

    http.POST(body);
    http.end();
}

int calibLoadTable(CalibPoint* table, int maxPoints) {
    for (int i = 0; i < maxPoints; i++) table[i].valid = false;
    if (WiFi.status() != WL_CONNECTED) return 0;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    String url = String(BACKEND_BASE) + "/api/calib/table?machine_id=" + MACHINE_ID;
    http.begin(client, url);
    http.setTimeout(5000);

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
        Serial.printf("[CALIB] load table failed: %d\n", code);
    }
    http.end();
    return loaded;
}

} // namespace net
