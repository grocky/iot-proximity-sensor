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
const static int BAUD_RATE = 9600;

const static unsigned int SENSOR_ECHO_PIN = D1;
const static unsigned int SENSOR_TRIGGER_PIN = D2;
const static unsigned long SENSOR_TIMEOUT_MS = 50 * 1000;
Ultrasonic ultrasonic(SENSOR_TRIGGER_PIN, SENSOR_ECHO_PIN, SENSOR_TIMEOUT_MS);

AsyncDelay triggerDelay;

struct SensorState {
    u_int distance;
    int numIntervals;
    bool lightState;
} sensorState;

ProjectConfiguration* config;

void setup() {
    Serial.begin(BAUD_RATE);
    delay(ONE_SECOND);
    Serial.println("Initializing...");

    config = new ProjectConfiguration(BAUD_RATE);
    config->setup();

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
    config->handle();

    if (sensorState.lightState) {
        Serial.print(".");
    } else {
        sensorState.distance = ultrasonic.read(INC);
        sensorState.numIntervals = sensorState.distance <= config->sonar.triggerDistanceInches
                                   ? sensorState.numIntervals + 1
                                   : 0;

        Serial.printf("distance: %d inches, intervals: %d\r\n", sensorState.distance, sensorState.numIntervals);
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
