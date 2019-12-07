#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AutoConnect.h>

ESP8266WebServer Server;
AutoConnect Portal(Server);

static const uint8  PIN_SENSOR_1 = D5;

void rootPage() {
    char content[] = "Hello, world";
    Server.send(200, "text/plain", content);
}

void setup() {
    pinMode(PIN_SENSOR_1, INPUT_PULLUP);
    Serial.begin(9600);

    delay(1000);
    Serial.println();

    Server.on("/", rootPage);

    if (Portal.begin()) {
        Serial.println("WiFi connected: " + WiFi.localIP().toString());
    }
}

bool isWaterHigh(int sensorPin) {
    return digitalRead(sensorPin) == 1;
}

void loop() {
    if (isWaterHigh(PIN_SENSOR_1)) {
        Serial.println("Water good");
    } else {
        Serial.println("Water low") ;
    }

    Portal.handleClient();

    delay(1000);
}
