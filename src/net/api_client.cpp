#include "api_client.h"
#include "../config.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>

namespace net {

void sendSet(uint8_t reps, uint16_t romMm, uint32_t durationMs, int weightKg, const char* nfcUid) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[API] WiFi not connected, skipping POST");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();  // skip cert verification (ok for this use case)

    HTTPClient http;
    http.begin(client, BACKEND_URL);
    http.addHeader("Content-Type", "application/json");

    uint32_t durationSec = durationMs / 1000;
    String body = "{\"machine_id\":\"";
    body += MACHINE_ID;
    body += "\",\"reps\":";
    body += reps;
    body += ",\"rom_mm\":";
    body += romMm;
    body += ",\"duration_sec\":";
    body += durationSec;
    if (weightKg >= 0) {
        body += ",\"weight_kg\":";
        body += weightKg;
    }
    if (nfcUid && nfcUid[0]) {
        body += ",\"nfc_uid\":\"";
        body += nfcUid;
        body += "\"";
    }
    body += "}";

    int code = http.POST(body);
    if (code > 0) {
        Serial.print("[API] POST /set -> ");
        Serial.println(code);
    } else {
        Serial.print("[API] POST failed: ");
        Serial.println(http.errorToString(code));
    }
    http.end();
}

} // namespace net
