//
// Created by Rocky Gray on 10/6/19.
//
#include <Arduino.h>

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <Ultrasonic.h>

#include <Config.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN  13
#endif

Ultrasonic ultrasonic(D1);
u_int distance;
u_int numIntervals;

const static unsigned int ONE_SECOND = 1000;
const static unsigned int SECONDS = ONE_SECOND;
const static unsigned int ONE_MINUTE = ONE_SECOND * 60;
const static unsigned int MINUTES = ONE_MINUTE;
const static unsigned int LIGHT_TIMEOUT = 5 * SECONDS;

bool lightState = false;
Config* C;

void setup() {
    Serial.begin(9600);
    delay(ONE_SECOND);
    Serial.println("Initializing...");
    C = new Config();

    Bleeper
        .verbose()
        .configuration
            .set(C)
            .addObserver(new SettingsCallbackObserver([](const ConfigurationPropertyChange value) {
                Serial.begin(9600);
                delay(10);
                Serial.println("Configuration " + value.key + " changed from " + value.oldValue + " to " + value.newValue);
                delay(10);
            }), {&C->sonar.triggerDistance})
            .done()
        .configurationInterface
            .addDefaultWebServer()
            .done()
        .storage
            .set(new SPIFFSStorage()) // SPIFFS
            .done()
        .init();

    distance = 128;
    numIntervals = 0;
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("Opening configuration portal");
    digitalWrite(LED_BUILTIN, LOW);
    WiFiManager wifiManager;

    if (!wifiManager.autoConnect()) {
        Serial.println("failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(1000);
    }

    Serial.println("WiFi connection completed");
    digitalWrite(LED_BUILTIN, HIGH);

    int connResult = WiFi.waitForConnectResult();
    Serial.println(connResult);

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("failed to connect, finishing setup anyway");
    } else {
        Serial.printf("local ip: %s\r\n", WiFi.localIP().toString().c_str());
    }

    delay(ONE_SECOND);
}

void loop() {
    Bleeper.handle();

    distance = ultrasonic.read(INC);
    Serial.printf("distance: %d inches\r\n", distance);

    numIntervals = distance <= C->sonar.triggerDistance
            ? numIntervals + 1
            : 0;

    if (numIntervals >= 3) {
        Serial.println("turning light on");
        lightState = true;
        digitalWrite(LED_BUILTIN, LOW);
        delay(LIGHT_TIMEOUT);
    }

    if (lightState) {
        Serial.println("turning light off");
        lightState = false;
        digitalWrite(LED_BUILTIN, HIGH);
    }

    delay(100);
}
