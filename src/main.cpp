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

#ifndef LED_BUILTIN
#define LED_BUILTIN  13
#endif

const static unsigned int ONE_SECOND = 1000;
const static unsigned int HALF_SECOND = 500;

const static int BAUD_RATE = 250000;

const static unsigned int LIGHTS_BLUE_PIN = D2;
const static unsigned int LIGHTS_RED_PIN = D3;
const static unsigned int LIGHTS_GREEN_PIN = D4;

AsyncDelay* triggerDelay;

ProjectConfiguration* config;
Ultrasonic* ultrasonic;
Servo armServo;

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
    showAnalogRGB(CRGB::Red);
    delay(500);

    showAnalogRGB(CRGB::Green);
    delay(500);

    showAnalogRGB(CRGB::Blue);
    delay(500);

    showAnalogRGB(CRGB::Black);
    delay(500);
}

void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println();
    Serial.println("Initializing...");

    pinMode(LIGHTS_GREEN_PIN, OUTPUT);
    pinMode(LIGHTS_BLUE_PIN, OUTPUT);
    pinMode(LIGHTS_RED_PIN, OUTPUT);

    colorBars();
    delay(500);

    pinMode(LED_BUILTIN, OUTPUT);

    setupWifi();
    setupConfiguration();

    triggerDelay = new AsyncDelay(config->sonar.triggerTimeoutSeconds * ONE_SECOND, AsyncDelay::MILLIS);

    Log.begin(config->log.logLevel, &Serial, false);

    delay(2 * ONE_SECOND);
}

void loop() {
    config->handle();

    showAnalogRGB(CRGB::GhostWhite);
    delay(2000);
    showAnalogRGB(CRGB::Coral);
    delay(2000);
    showAnalogRGB(CRGB::WhiteSmoke);
    delay(2000);
    showAnalogRGB(CRGB::Teal);
    delay(2000);
    showAnalogRGB(CRGB::White);
    delay(2000);
    showAnalogRGB(CRGB::SpringGreen);
    delay(2000);
    showAnalogRGB(CRGB::Black);
    delay(5000);

}
