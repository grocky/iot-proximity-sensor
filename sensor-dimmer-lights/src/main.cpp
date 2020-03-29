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
#include <FastLED.h>

#include <ProjectConfiguration.h>
#include <PirSensor.h>
#include <PubSubClient.h>

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

const static char* MQTT_MOTION_AVAILABILITY_TOPIC = "home/upstairs/stair-lights";
const static char* MQTT_MOTION_STATE_TOPIC = "home/upstairs/stair-lights/motion";
const static char* MQTT_LIGHT_STATE_TOPIC = "home/upstairs/stair-lights/lights";

AsyncDelay* triggerDelay;

ProjectConfiguration* config;
Ultrasonic* ultrasonic;

PirSensor* motion;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool mqttEnabled = false;

bool isLightOn = false;

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

    CallbackFn updateLightDuration = [](const ConfigurationPropertyChange value) {
        if (value.key == "lights.durationSeconds") {
            triggerDelay = new AsyncDelay(config->lights.durationSeconds * ONE_SECOND, AsyncDelay::MILLIS);
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

void colorToString(CRGB* color, char* buffer) {
    sprintf(buffer, "R: %X G: %X B: %X", color->red, color->green, color->blue);
}

void triggerLights(bool isOn) {
    isLightOn = isOn;
    CRGB color = isOn ? CRGB::GhostWhite : CRGB::Black;

    Serial.print("Setting lights to ");

    char colorString[100];
    colorToString(&color, colorString);
    Serial.println(colorString);

    showAnalogRGB(color);
}

void mqttTopicCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    std::string topicString(topic);

    if (topicString == MQTT_LIGHT_STATE_TOPIC) {
        bool value = (char)payload[0] == '1';
        return triggerLights(value);
    }
}

bool mqttReconnect() {
    Serial.print("Attempting MQTT connection...");

    mqttClient.setServer(config->mqtt.serverHost.c_str(), config->mqtt.serverPort);
    mqttClient.setCallback(mqttTopicCallback);

    // Create a random client ID
    char* clientId = "upstairs-motion-lights";

    // Attempt to connect
    if (!mqttClient.connect(clientId, MQTT_MOTION_AVAILABILITY_TOPIC, 0, true, "offline")) {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
        return false;
    }

    Serial.println("connected");
    // Once connected, publish current state...
    mqttClient.publish(MQTT_MOTION_AVAILABILITY_TOPIC, "online");
    mqttClient.publish(MQTT_MOTION_STATE_TOPIC, "0");
    mqttClient.publish(MQTT_LIGHT_STATE_TOPIC, "0");
    // ... and resubscribe
    mqttClient.subscribe(MQTT_LIGHT_STATE_TOPIC);

    return true;
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

    triggerDelay = new AsyncDelay(config->lights.durationSeconds * ONE_SECOND, AsyncDelay::MILLIS);

    motion = new PirSensor(MOTION_SENSOR_PIN, 2, false, false);
    motion->begin();

    Log.begin(config->log.logLevel, &Serial, false);

    colorBars();
    Log.notice("Initialization complete!\n");
}

void loop() {
    config->handle();

    if (!mqttClient.connected()) {
        mqttEnabled = config->mqtt.serverHost != "";
        if (!mqttEnabled || !mqttReconnect()) {
            return;
        }
    }

    mqttClient.loop();

    int motionValue = motion->sampleValue();

    if (motionValue == 0) {
        Log.notice("Motion detection off\n");
        mqttClient.publish(MQTT_MOTION_STATE_TOPIC, "0");
    }

    if (motionValue == 1) {
        Log.notice("Motion detected\n");
        mqttClient.publish(MQTT_MOTION_STATE_TOPIC, "1");
    }
}
