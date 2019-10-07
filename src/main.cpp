//
// Created by Rocky Gray on 10/6/19.
//
#include <Arduino.h>
#include <Ultrasonic.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN  13
#endif

Ultrasonic ultrasonic(D1);
uint distance;

void setup() {
    Serial.begin(9600);
    Serial.println("Initializing...");
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    distance = ultrasonic.read(INC);
    Serial.printf("Turning LED off: %d\r\n", distance);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

    distance = ultrasonic.read(INC);
    Serial.printf("Turning LED on: %d\r\n", distance);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
