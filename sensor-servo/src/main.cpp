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

const static unsigned int SERVO_PIN = D4;

AsyncDelay* triggerDelay;
AsyncDelay* sensorReadDelay;

struct SensorState {
    int numIntervals = 0;
    bool motionDetected = false;
    u_int servoAngle = 90;
    bool startServo = true;
    bool servoInProgress = false;
} sensorState;

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

    CallbackFn updateServo = [](const ConfigurationPropertyChange value) {
        if (value.key == "servo.angle") {
            sensorState.servoAngle = value.newValue.toInt();
            sensorState.startServo = true;
        }
    };

    config->addObserver(new SettingsCallbackObserver(updateLogger));
    config->addObserver(new SettingsCallbackObserver(updateServo));
}

void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println();
    Serial.println("Initializing...");

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SERVO_PIN, OUTPUT);

    setupWifi();
    setupConfiguration();
    ultrasonic = new Ultrasonic(SENSOR_TRIGGER_PIN, SENSOR_ECHO_PIN, SENSOR_TIMEOUT_MS);

    armServo.attach(SERVO_PIN);
    sensorState.servoAngle = config->servo.angle;
    armServo.write(sensorState.servoAngle);

    sensorReadDelay = new AsyncDelay(config->sonar.intervalDelayMilliseconds, AsyncDelay::MILLIS);
    triggerDelay = new AsyncDelay(config->sonar.triggerTimeoutSeconds * ONE_SECOND, AsyncDelay::MILLIS);

    Log.begin(config->log.logLevel, &Serial, false);

    delay(2 * ONE_SECOND);
}

SensorState handleSensorRead(SensorState prev) {
    SensorState next = prev;

    if (sensorReadDelay->isExpired()) {
        if (!prev.motionDetected) {
            Log.verbose("sensorReadDelay: delay: %l ms, expiry: %l, duration: %l ms, isExpired: %d\r\n",
                        sensorReadDelay->getDelay(), sensorReadDelay->getExpiry(), sensorReadDelay->getDuration(), sensorReadDelay->isExpired());

            int distance = (int)ultrasonic->read(INC);

            // With the JSN-SR04T distance sensor there's a bit of noise when reading.
            if (!distance) {
                return next;
            }

            next.numIntervals = distance <= config->sonar.triggerDistanceInches
                                       ? prev.numIntervals + 1
                                       : 0;

            Log.trace("distance: %d inches, intervals: %d\r\n", distance, next.numIntervals);
        } else {
            Log.trace(".");
        }

        // Using start to allow config updates...
        sensorReadDelay->start(config->sonar.intervalDelayMilliseconds, AsyncDelay::MILLIS);
    }

    return next;
}

SensorState handleMotionDetection(SensorState prev) {
    SensorState next = prev;

    if (prev.numIntervals >= config->sonar.triggerIntervals) {
        Log.trace("turning light on\r\n");
        next.motionDetected = true;
        digitalWrite(LED_BUILTIN, LOW);
        next.numIntervals = 0;

        // Using start to allow config updates...
        triggerDelay->start(config->sonar.triggerTimeoutSeconds * ONE_SECOND, AsyncDelay::MILLIS);
    }

    if (prev.motionDetected && triggerDelay->isExpired()) {
        Log.verbose("triggerDelay: delay: %l ms, expiry: %l, duration: %l ms, isExpired: %d\r\n",
                    triggerDelay->getDelay(), triggerDelay->getExpiry(), triggerDelay->getDuration(), triggerDelay->isExpired());

        Log.trace("turning light off\r\n");
        next.motionDetected = false;
        digitalWrite(LED_BUILTIN, HIGH);
    }

    return next;
}

void executeServoSequence(Servo servo) {
    servo.write(150);
    delay(3 * ONE_SECOND);
    servo.write(60);
    delay(2 * ONE_SECOND);
}

SensorState handleServoUpdate(SensorState prev) {
    SensorState next = prev;
    if (prev.motionDetected && !prev.servoInProgress) {
        next.startServo = true;
        next.servoInProgress = true;
        Log.verbose("Updating servo state: nextState: starting, %d, servoInProgress: %d, startServo: %d, angle: %d\r\n", next.motionDetected, next.servoInProgress, next.startServo, next.servoAngle);
    } else if (!prev.motionDetected && prev.servoInProgress) {
        next.servoInProgress = false;
        Log.verbose("Updating servo state: nextState: stopped, %d, servoInProgress: %d, startServo: %d, angle: %d\r\n", next.motionDetected, next.servoInProgress, next.startServo, next.servoAngle);
    } else if (prev.startServo) {
        executeServoSequence(armServo);
        next.servoInProgress = true;
        next.startServo = false;
        Log.verbose("Updating servo state: nextState: running, %d, servoInProgress: %d, startServo: %d, angle: %d\r\n", next.motionDetected, next.servoInProgress, next.startServo, next.servoAngle);
    }
    return next;
}

void loop() {
    config->handle();
    sensorState = handleSensorRead(sensorState);
    sensorState = handleMotionDetection(sensorState);
    sensorState = handleServoUpdate(sensorState);
}
