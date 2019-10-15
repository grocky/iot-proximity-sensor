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
int distance;
int numIntervals;

const static unsigned int ONE_SECOND = 1000;

bool lightState = false;
Config* C;

void setup() {
    Serial.begin(9600);
    delay(ONE_SECOND);
    Serial.println("Initializing...");
    C = new Config();
    std::function<void(const ConfigurationPropertyChange)> logConfigChange = [](const ConfigurationPropertyChange value) {
        Serial.println("Configuration " + value.key + " changed from " + value.oldValue + " to " + value.newValue);
    };

    Bleeper
        .verbose()
        .configuration
            .set(C)
            .addObserver(new SettingsCallbackObserver(logConfigChange), {
                &C->sonar.triggerDistanceInches,
                &C->sonar.triggerIntervals,
                &C->sonar.intervalDelayMilliseconds,
                &C->sonar.triggerTimeoutSeconds
            })
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

    numIntervals = distance <= C->sonar.triggerDistanceInches
            ? numIntervals + 1
            : 0;

    if (numIntervals >= C->sonar.triggerIntervals) {
        Serial.println("turning light on");
        lightState = true;
        digitalWrite(LED_BUILTIN, LOW);
        delay(C->sonar.triggerTimeoutSeconds * ONE_SECOND);
    }

    if (lightState) {
        Serial.println("turning light off");
        lightState = false;
        digitalWrite(LED_BUILTIN, HIGH);
    }

    delay(C->sonar.intervalDelayMilliseconds);
}
