//
// Created by Rocky Gray on 10/6/19.
//
#include <Arduino.h>
#include <Ultrasonic.h>

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

const static unsigned int PROXIMITY_THRESHOLD_INCHES = 10;
bool lightState = false;

void setup() {
    Serial.begin(9600);
    delay(ONE_SECOND);
    Serial.println("Initializing...");
    distance = 128;
    numIntervals = 0;
    pinMode(LED_BUILTIN, OUTPUT);
    delay(ONE_SECOND * .5);
}

void loop() {
    distance = ultrasonic.read(INC);
    Serial.printf("distance: %d inches\r\n", distance);

    numIntervals = distance < PROXIMITY_THRESHOLD_INCHES
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
