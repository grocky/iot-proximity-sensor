//
// Created by Rocky Gray on 10/6/19.
//
#include <Arduino.h>

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include <ArduinoLog.h>
#include <AsyncDelay.h>
#include <Ultrasonic.h>
#include <WiFiManager.h>

#include <ProjectConfiguration.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN  13
#endif

const static unsigned int ONE_SECOND = 1000;
const static unsigned int HALF_SECOND = 500;

const static int BAUD_RATE = 250000;

const static unsigned int SENSOR_ECHO_PIN = D1;
const static unsigned int SENSOR_TRIGGER_PIN = D2;
const static unsigned long SENSOR_TIMEOUT_MS = 20 * 1000;

AsyncDelay* triggerDelay;
AsyncDelay* sensorReadDelay;

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
    Serial.println();
    Serial.println("Initializing...");
    pinMode(LED_BUILTIN, OUTPUT);

    config = new ProjectConfiguration(BAUD_RATE);
    ultrasonic = new Ultrasonic(SENSOR_TRIGGER_PIN, SENSOR_ECHO_PIN, SENSOR_TIMEOUT_MS);

    setupWifi();

    sensorReadDelay = new AsyncDelay(config->sonar.intervalDelayMilliseconds, AsyncDelay::MILLIS);
    triggerDelay = new AsyncDelay(config->sonar.triggerTimeoutSeconds * ONE_SECOND, AsyncDelay::MILLIS);

    Log.begin(config->log.logLevel, &Serial, false);

    delay(2 * ONE_SECOND);
}


void loop() {
    config->handle();

    // sensorHandler
    if (sensorReadDelay->isExpired()) {
        if (!sensorState.lightState) {
            Log.verbose("sensorReadDelay: delay: %l ms, expiry: %l, duration: %l ms, isExpired: %d\r\n",
                          sensorReadDelay->getDelay(), sensorReadDelay->getExpiry(), sensorReadDelay->getDuration(), sensorReadDelay->isExpired());

            int distance = (int)ultrasonic->read(INC);

            // With the JSN-SR04T distance sensor there's a bit of noise when reading.
            if (!distance) {
                return;
            }

            sensorState.numIntervals = distance <= config->sonar.triggerDistanceInches
                                       ? sensorState.numIntervals + 1
                                       : 0;

            Log.trace("distance: %d inches, intervals: %d\r\n", distance, sensorState.numIntervals);
        } else {
            Log.trace(".");
        }

        // Using start to allow config updates...
        sensorReadDelay->start(config->sonar.intervalDelayMilliseconds, AsyncDelay::MILLIS);
    }

    // triggerHandler
    if (sensorState.numIntervals >= config->sonar.triggerIntervals) {
        Log.trace("turning light on");
        sensorState.lightState = true;
        digitalWrite(LED_BUILTIN, LOW);
        sensorState.numIntervals = 0;

        // Using start to allow config updates...
        triggerDelay->start(config->sonar.triggerTimeoutSeconds * ONE_SECOND, AsyncDelay::MILLIS);
    }

    if (sensorState.lightState && triggerDelay->isExpired()) {
        Log.verbose("triggerDelay: delay: %l ms, expiry: %l, duration: %l ms, isExpired: %d",
                    triggerDelay->getDelay(), triggerDelay->getExpiry(), triggerDelay->getDuration(), triggerDelay->isExpired());

        Log.trace("turning light off");
        sensorState.lightState = false;
        digitalWrite(LED_BUILTIN, HIGH);
    }
}
