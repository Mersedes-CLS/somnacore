#include "wifi_manager.h"
#include "../config.h"
#include <Arduino.h>
#include <WiFi.h>

namespace net {

void wifiConnect() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS) {
        delay(WIFI_RETRY_MS);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void wifiCheck() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost, reconnecting...");
        wifiConnect();
    }
}

}  // namespace net
