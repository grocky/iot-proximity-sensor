//
// Created by Rocky Gray on 10/6/19.
//
#include <Arduino.h>

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

#include <ArduinoLog.h>
#include <AsyncDelay.h>
#include <Ultrasonic.h>
#include <WiFiManager.h>
#include <FastLED.h>

#include <ProjectConfiguration.h>
#include <PirSensor.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN  13
#endif

const static unsigned int ONE_SECOND = 1000;
const static unsigned int HALF_SECOND = 500;

const static int BAUD_RATE = 250000;

const static unsigned int LIGHTS_BLUE_PIN = D2;
const static unsigned int LIGHTS_RED_PIN = D3;
const static unsigned int LIGHTS_GREEN_PIN = D4;
const static unsigned int MOTION_SENSOR_PIN = D5;

AsyncDelay* triggerDelay;

ProjectConfiguration* config;
Ultrasonic* ultrasonic;

PirSensor* motion;

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

void setupConfiguration() {
    config = new ProjectConfiguration(BAUD_RATE);

    CallbackFn updateLogger = [](const ConfigurationPropertyChange value) {
        if (value.key == "log.logLevel") {
            Log.begin(value.newValue.toInt(), &Serial, false);
        }
    };

    config->addObserver(new SettingsCallbackObserver(updateLogger));
}

void showAnalogRGB(const CRGB& rgb) {
    analogWrite(LIGHTS_RED_PIN, rgb.red );
    analogWrite(LIGHTS_GREEN_PIN, rgb.green );
    analogWrite(LIGHTS_BLUE_PIN, rgb.blue );
}

void colorBars() {
    CRGB colors[8] = {
        CRGB::Red,
        CRGB::Orange,
        CRGB::Yellow,
        CRGB::Green,
        CRGB::Blue,
        CRGB::Indigo,
        CRGB::Violet,
        CRGB::Black
    };

    for(const CRGB& color : colors) {
        showAnalogRGB(color);
        delay(200);
    }
}

void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println();
    Serial.println("Initializing...");

    pinMode(LIGHTS_GREEN_PIN, OUTPUT);
    pinMode(LIGHTS_BLUE_PIN, OUTPUT);
    pinMode(LIGHTS_RED_PIN, OUTPUT);

    colorBars();

    pinMode(LED_BUILTIN, OUTPUT);

    setupWifi();
    setupConfiguration();

    triggerDelay = new AsyncDelay(config->sonar.triggerTimeoutSeconds * ONE_SECOND, AsyncDelay::MILLIS);

    motion = new PirSensor(MOTION_SENSOR_PIN, 2, false, false);
    motion->begin();

    Log.begin(config->log.logLevel, &Serial, false);

    colorBars();
    Log.notice("Initialization complete!\n");
}

void loop() {
    config->handle();

    int motionValue = motion->sampleValue();

    if (motionValue < 0 ) {
        return;
    }

    if (motionValue == 0) {
        Log.notice("Motion detection off\n");
    }

    if (motionValue == 1) {
        Log.notice("Motion detected\n");
        showAnalogRGB(CRGB::GhostWhite);
        delay(5000);
        showAnalogRGB(CRGB::Black);
    }
}
