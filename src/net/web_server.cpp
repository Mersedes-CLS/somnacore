#include "web_server.h"
#include "../config.h"
#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

namespace net {

static WebServer server(HTTP_PORT);
static volatile uint16_t* pDistance = nullptr;

static void handleRoot() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to open index.html");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

static void handleDistance() {
    uint16_t d = pDistance ? *pDistance : 0;
    String json = "{\"distance_mm\":" + String(d) + ",\"t\":" + String(millis()) + "}";
    server.send(200, "application/json", json);
}

void webServerInit(volatile uint16_t* distancePtr) {
    pDistance = distancePtr;

    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed!");
    }

    server.on("/", handleRoot);
    server.on("/distance", handleDistance);
    server.begin();
    Serial.println("Web server started");
}

void webServerHandle() {
    server.handleClient();
}

}  // namespace net
