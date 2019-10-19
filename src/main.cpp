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
#include <AsyncDelay.h>

#include <ProjectConfiguration.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN  13
#endif

const static unsigned int ONE_SECOND = 1000;
const static unsigned int HALF_SECOND = 500;

const static int BAUD_RATE = 9600;

const static unsigned int SENSOR_ECHO_PIN = D1;
const static unsigned int SENSOR_TRIGGER_PIN = D2;
const static unsigned long SENSOR_TIMEOUT_MS = 20 * 1000;

AsyncDelay triggerDelay;

struct SensorState {
    int numIntervals;
    bool lightState;
} sensorState;

ProjectConfiguration* config;
Ultrasonic* ultrasonic;

void setupWifi() {
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
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(HALF_SECOND);
    Serial.flush();

    Serial.println("Initializing...");
    pinMode(LED_BUILTIN, OUTPUT);

    config = new ProjectConfiguration(BAUD_RATE);
    ultrasonic = new Ultrasonic(SENSOR_TRIGGER_PIN, SENSOR_ECHO_PIN, SENSOR_TIMEOUT_MS);

    setupWifi();

    delay(2 * ONE_SECOND);
}

void loop() {
    config->handle();

    int distance = (int)ultrasonic->read(INC);

    // With the JSN-SR04T distance sensor there's a bit of noise when reading.
    if (!distance) {
        return;
    }

    if (sensorState.lightState) {
        Serial.print(".");
    } else {
        sensorState.numIntervals = distance <= config->sonar.triggerDistanceInches
                                   ? sensorState.numIntervals + 1
                                   : 0;

        Serial.printf("distance: %d inches, intervals: %d\r\n", distance, sensorState.numIntervals);
    }

    if (sensorState.numIntervals >= config->sonar.triggerIntervals) {
        Serial.println("turning light on");
        sensorState.lightState = true;
        digitalWrite(LED_BUILTIN, LOW);
        sensorState.numIntervals = 0;
        triggerDelay.start(config->sonar.triggerTimeoutSeconds * ONE_SECOND, AsyncDelay::MILLIS);
    }

    if (sensorState.lightState && triggerDelay.isExpired()) {
        Serial.println("turning light off");
        sensorState.lightState = false;
        digitalWrite(LED_BUILTIN, HIGH);
    }

    delay(config->sonar.intervalDelayMilliseconds);
}
